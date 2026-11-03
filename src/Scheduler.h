#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Config.h"
#include "Process.h"
#include "ProcessFactory.h"

struct RunningEntry {
    std::shared_ptr<Process> process;
    int                      core = -1;
};

// Everything the console / report needs, captured under one lock.
struct SchedulerSnapshot {
    int                                   totalCores = 0;
    int                                   coresUsed = 0;
    int                                   coresAvailable = 0;
    double                                utilizationPercent = 0.0;
    std::vector<RunningEntry>             running;
    std::vector<std::shared_ptr<Process>> finished;
};

// Real-time scheduler: one tick thread + one worker thread per emulated CPU core.
class Scheduler {
public:
    Scheduler(const Config& cfg, ProcessFactory& factory);
    ~Scheduler();

    void start();   // spawns tick thread + core threads
    void stop();    // joins everything (safe to call twice)

    // "scheduler-start" / "scheduler-stop": toggles auto-generation of dummy processes.
    void setGenerating(bool on) { generating_ = on; }
    bool isGenerating() const { return generating_; }

    // Put a new process into the ready queue (from screen -s or the generator).
    void addProcess(std::shared_ptr<Process> p);

    // Returns the process if it exists and has NOT finished, otherwise nullptr.
    // (used by "screen -r <name>")
    std::shared_ptr<Process> findActiveProcess(const std::string& name) const;

    SchedulerSnapshot snapshot() const;
    std::uint64_t     currentTick() const;

private:
    void          tickLoop();
    void          coreLoop(int coreId);
    std::uint64_t waitForTick(std::uint64_t lastSeen);
    std::shared_ptr<Process> takeNextReady();  // pops the front of the ready queue (or nullptr)

    Config          cfg_;
    ProcessFactory& factory_;

    // tick clock
    mutable std::mutex      tickMtx_;
    std::condition_variable tickCv_;
    std::uint64_t           tick_ = 0;
    std::atomic<bool>       running_{false};
    std::atomic<bool>       generating_{false};

    std::uint64_t           coresDone_ = 0; 
    std::condition_variable doneCv_; 

    // scheduler state (guarded by stateMtx_)
    mutable std::mutex                    stateMtx_;
    std::deque<std::shared_ptr<Process>>  readyQueue_;
    std::vector<std::shared_ptr<Process>> runningPerCore_;  // index = core id
    std::vector<std::shared_ptr<Process>> finished_;
    std::vector<std::shared_ptr<Process>> allProcesses_;

    std::thread              tickThread_;
    std::vector<std::thread> coreThreads_;
};
