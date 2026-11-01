#include "Process.h"

#include <algorithm>
#include <limits>

#include "Utils.h"

Process::Process(int id, std::string name, std::vector<Instruction> flatProgram)
    : id_(id),
      name_(std::move(name)),
      createdAt_(timestampNow()),
      program_(std::move(flatProgram)) {
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
    // operator[] default-constructs to 0 => "auto-declared with a value of 0"
    return vars_[op.varName];
}

void Process::executeNext(int coreId, std::uint64_t currentTick) {
    std::lock_guard<std::mutex> lk(mtx_);
    if (finished_) return;
    if (currentTick < sleepUntilTick_) return;  // still asleep, core should be relinquished
    if (pc_ >= program_.size()) { finished_ = true; return; }

    lastCore_ = coreId;
    const Instruction& ins = program_[pc_];

    switch (ins.type) {
        case InstructionType::PRINT:
            // TODO(Member 3): build msg (+ optional variable value) and push a LogEntry
            //   { timestampNow(), coreId, text } into logs_.
            break;
        case InstructionType::DECLARE:
            // TODO(Member 3): vars_[ins.target] = ins.lhs.literal;
            break;
        case InstructionType::ADD:
            // TODO(Member 3): vars_[ins.target] = clampAdd(resolve(ins.lhs), resolve(ins.rhs));
            break;
        case InstructionType::SUBTRACT:
            // TODO(Member 3): vars_[ins.target] = clampSub(resolve(ins.lhs), resolve(ins.rhs));
            break;
        case InstructionType::SLEEP:
            // TODO(Member 3): sleepUntilTick_ = currentTick + ins.sleepTicks;
            //   (Member 2's scheduler must notice isSleeping() and free the core.)
            break;
        case InstructionType::FOR:
            // Not expected here: flattenProgram() should have unrolled all FORs.
            break;
    }

    ++pc_;
    if (pc_ >= program_.size()) finished_ = true;
    // TODO(Member 3/4): record a finish timestamp if the report needs it.
}
