#include "Process.h"

#include <algorithm>
#include <limits>

#include "Utils.h"

Process::Process(int id, std::string name, std::vector<Instruction> flatProgram)
    : id_(id),
      name_(std::move(name)),
      createdAt_(timestampNow()),
      program_(std::move(flatProgram)),
      totalLines_(program_.size()) {
    if (program_.empty()) finished_ = true;
}

std::size_t Process::currentLine() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return pc_;
}

bool Process::isFinished() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return finished_;
}

bool Process::isSleeping(std::uint64_t currentTick) const {
    std::lock_guard<std::mutex> lk(mtx_);
    return !finished_ && currentTick < sleepUntilTick_;
}

int Process::lastCore() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return lastCore_;
}

std::vector<LogEntry> Process::logs() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return logs_;
}

std::uint16_t Process::clampAdd(std::uint32_t a, std::uint32_t b) {
    const std::uint32_t sum = a + b;
    return static_cast<std::uint16_t>(
        std::min<std::uint32_t>(sum, std::numeric_limits<std::uint16_t>::max()));
}

std::uint16_t Process::clampSub(std::int32_t a, std::int32_t b) {
    const std::int32_t diff = a - b;
    return static_cast<std::uint16_t>(std::max<std::int32_t>(diff, 0));
}

std::uint16_t Process::resolve(const Operand& op) {
    if (!op.isVariable) return op.literal;
    // operator[] default-constructs to 0 -> "auto-declared with a value of 0"
    return vars_[op.varName];
}

void Process::markFinished() {
    finished_ = true;
    // spec: variables are released when the process finishes. Also free the instruction list
    // (totalLines_ keeps the line count for process-smi / reports).
    vars_.clear();
    program_.clear();
    program_.shrink_to_fit();
}

void Process::executeNext(int coreId, std::uint64_t currentTick,
                          std::uint64_t delaysPerExec) {
    std::lock_guard<std::mutex> lk(mtx_);
    if (finished_) return;
    if (currentTick < sleepUntilTick_) return;
    if (delayLeft_ > 0) {
        --delayLeft_;
        return;
    }
    if (pc_ >= program_.size()) { markFinished(); return; }

    lastCore_ = coreId;
    const Instruction& ins = program_[pc_];

    switch (ins.type) {
        case InstructionType::PRINT: {
            std::string text = ins.msg;
            if (!ins.printVar.empty()) text += std::to_string(vars_[ins.printVar]);
            logs_.push_back({timestampNow(), coreId, std::move(text)});
            break;
        }
        case InstructionType::DECLARE:
            vars_[ins.target] = ins.lhs.literal;
            break;
        case InstructionType::ADD: {
            const std::uint16_t a = resolve(ins.lhs);
            const std::uint16_t b = resolve(ins.rhs);
            vars_[ins.target] = clampAdd(a, b);
            break;
        }
        case InstructionType::SUBTRACT: {
            const std::uint16_t a = resolve(ins.lhs);
            const std::uint16_t b = resolve(ins.rhs);
            vars_[ins.target] = clampSub(a, b);
            break;
        }
        case InstructionType::SLEEP:
            sleepUntilTick_ = currentTick + ins.sleepTicks;
            break;
        case InstructionType::FOR:
            break;
    }

    ++pc_;
    delayLeft_ = delaysPerExec;
    if (pc_ >= program_.size()) markFinished();
}
