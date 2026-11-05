#include "Scheduler.h"

#include <chrono>

namespace {
// Real-time length of one CPU tick. 
constexpr int kGenTickMs = 5;
constexpr int kIdleSleepMs = 1;
}

Scheduler::Scheduler(const Config& cfg, ProcessFactory& factory)
    : cfg_(cfg), factory_(factory), runningPerCore_(static_cast<std::size_t>(cfg.numCpu)) {}

Scheduler::~Scheduler() { stop(); }

void Scheduler::start() {
    {
        std::lock_guard<std::mutex> lk(tickMtx_);
        if (running_) return;
        running_ = true;
    }
    tickThread_ = std::thread(&Scheduler::tickLoop, this);
    for (int i = 0; i < cfg_.numCpu; ++i)
        coreThreads_.emplace_back(&Scheduler::coreLoop, this, i);
}

void Scheduler::stop() {
    {
        std::lock_guard<std::mutex> lk(tickMtx_);
        if (!running_) return;
        running_ = false;
    }
    tickCv_.notify_all();
    doneCv_.notify_all();
    if (tickThread_.joinable()) tickThread_.join();
    for (auto& t : coreThreads_)
        if (t.joinable()) t.join();
    coreThreads_.clear();
}

void Scheduler::addProcess(std::shared_ptr<Process> p) {
    std::lock_guard<std::mutex> lk(stateMtx_);
    allProcesses_.push_back(p);
    readyQueue_.push_back(std::move(p));
}

std::shared_ptr<Process> Scheduler::findActiveProcess(const std::string& name) const {
    std::lock_guard<std::mutex> lk(stateMtx_);
    for (const auto& p : allProcesses_)
        if (p->name() == name && !p->isFinished()) return p;
    return nullptr;
}

std::uint64_t Scheduler::currentTick() const {
    std::lock_guard<std::mutex> lk(tickMtx_);
    return tick_;
}

SchedulerSnapshot Scheduler::snapshot() const {
    std::lock_guard<std::mutex> lk(stateMtx_);
    SchedulerSnapshot s;
    s.totalCores = cfg_.numCpu;
    for (int core = 0; core < cfg_.numCpu; ++core) {
        if (runningPerCore_[core]) {
            ++s.coresUsed;
            s.running.push_back({runningPerCore_[core], core});
        }
    }
    s.coresAvailable = s.totalCores - s.coresUsed;
    s.utilizationPercent = s.totalCores ? 100.0 * s.coresUsed / s.totalCores : 0.0;
    s.finished = finished_;
    return s;
}

std::shared_ptr<Process> Scheduler::takeNextReady() {
    std::lock_guard<std::mutex> lk(stateMtx_);
    if (readyQueue_.empty()) return nullptr;
    auto p = readyQueue_.front();
    readyQueue_.pop_front();
    return p;
}

std::uint64_t Scheduler::waitForTick(std::uint64_t lastSeen) {
    std::unique_lock<std::mutex> lk(tickMtx_);
    tickCv_.wait(lk, [&] { return tick_ > lastSeen || !running_; });
    return tick_;
}

void Scheduler::tickLoop() {
    using Clock = std::chrono::steady_clock;
    const auto startTime = Clock::now();
    std::uint64_t genTick = 0;

    while (running_) {
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - startTime).count();
        const std::uint64_t genTarget = static_cast<std::uint64_t>(elapsedMs) / kGenTickMs;
        
        while (genTick < genTarget) {
            ++genTick;
            if(generating_ && cfg_.batchProcessFreq > 0 && genTick % cfg_.batchProcessFreq == 0) addProcess(factory_.createAuto());
        }

        bool idle;
        {
            std::lock_guard<std::mutex> lk(stateMtx_);
            idle = readyQueue_.empty();
            for (const auto& p: runningPerCore_)
                if (p) {idle = false; break; }
        }

        if (idle) {
            std::this_thread::sleep_for(std::chrono::milliseconds(kIdleSleepMs));
            continue;
        }
        std::uint64_t now;
        {
            std::lock_guard<std::mutex> lk(tickMtx_);
            now = ++tick_;
        }
        tickCv_.notify_all();

        // Barrier: wait until every core has finished this tick before starting the next one,
        // so no core can skip a CPU cycle (delay-per-exec / "one instruction per cycle").
        {
            std::unique_lock<std::mutex> lk(tickMtx_);
            doneCv_.wait(lk, [&] {
                return coresDone_ >= static_cast<std::uint64_t>(cfg_.numCpu) * now || !running_;
            });
        }
    }
}

void Scheduler::coreLoop(int coreId) {
    std::uint64_t lastTick = 0;
    std::uint64_t quantumUsed = 0;
    std::shared_ptr<Process> cur;
 
    // Caller must hold stateMtx_. Picks the first non-sleeping ready process (or none).
    auto dispatchLocked = [&](std::uint64_t tick) {
        cur = nullptr;
        for (auto it = readyQueue_.begin(); it != readyQueue_.end(); ++it) {
            if (!(*it)->isSleeping(tick)) {
                cur = *it;
                readyQueue_.erase(it);
                break;
            }
        }
        runningPerCore_[coreId] = cur;
        quantumUsed = 0;
    };
 
    while (running_) {
        lastTick = waitForTick(lastTick);
        if (!running_) break;
 
        // 1. Free core -> pick up a process.
        if (!cur) {
            std::lock_guard<std::mutex> lk(stateMtx_);
            dispatchLocked(lastTick);
        }
 
        if (cur) {
            // 2. One CPU cycle on this core.
            ++quantumUsed;
            // The process remembers its delay when preempted or moved to another core.
            cur->executeNext(coreId, lastTick, cfg_.delaysPerExec);
 
            // 3. Decide what happens to the process.
            const bool done     = cur->isFinished();
            const bool sleeping = !done && cur->isSleeping(lastTick);
            const bool expired  = !done && !sleeping && cfg_.scheduler == "rr" &&
                                  quantumUsed >= cfg_.quantumCycles;
 
            if (done || sleeping || expired) {
                // Release + requeue + next dispatch in one critical section so snapshot()
                // never sees an empty core while work is waiting.
                std::lock_guard<std::mutex> lk(stateMtx_);
                if (done)
                    finished_.push_back(cur);
                else
                    readyQueue_.push_back(cur);  // sleeping (relinquish CPU) or RR preempt
                dispatchLocked(lastTick);
            }
        }
 
        // 4. Tell the tick thread this core is done with the tick (even when idle).
        {
            std::lock_guard<std::mutex> lk(tickMtx_);
            ++coresDone_;
        }
        doneCv_.notify_one();
    }
}
