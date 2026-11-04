#include "Process.h"

#include <cstdlib>
#include <iostream>

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

Instruction printInstruction() {
    Instruction ins;
    ins.type = InstructionType::PRINT;
    ins.msg = "test";
    return ins;
}

Instruction printVariableInstruction(const std::string& variable) {
    Instruction ins;
    ins.type = InstructionType::PRINT;
    ins.msg = "Value from: ";
    ins.printVar = variable;
    return ins;
}

}  // namespace

int main() {
    // One allocated cycle per turn, moving between cores: equivalent to quantum 1.
    Process delayed(1, "delayed", std::vector<Instruction>(3, printInstruction()));
    for (std::uint64_t tick = 1; tick <= 20; ++tick) {
        delayed.executeNext(static_cast<int>(tick % 4), tick, 10);
        expect(delayed.currentLine() == (tick <= 11 ? 1u : 2u),
               "ten busy-wait cycles must separate instructions across turns and cores");
    }

    // Another process on the same core starts without inheriting the first one's delay.
    Process other(2, "other", std::vector<Instruction>(2, printInstruction()));
    other.executeNext(0, 21, 10);
    expect(other.currentLine() == 1, "a new process must execute its first instruction immediately");

    // Time off the CPU does not consume busy-wait cycles.
    other.executeNext(1, 1000, 10);
    expect(other.currentLine() == 1, "waiting in the ready queue must not erase the delay");
    for (std::uint64_t tick = 1001; tick <= 1009; ++tick)
        other.executeNext(1, tick, 10);
    expect(other.currentLine() == 1, "the full delay must elapse before the next instruction");
    other.executeNext(2, 1010, 10);
    expect(other.isFinished(), "execution must resume after the remaining delay is consumed");

    Process noDelay(3, "no-delay", std::vector<Instruction>(3, printInstruction()));
    for (std::uint64_t tick = 1; tick <= 3; ++tick) {
        noDelay.executeNext(0, tick, 0);
        expect(noDelay.currentLine() == tick, "zero delay must allow one instruction per cycle");
    }
    expect(noDelay.isFinished(), "the final instruction must finish without an extra delay");

    Instruction declareX;
    declareX.type = InstructionType::DECLARE;
    declareX.target = "x";
    declareX.lhs.literal = 42;
    Process variablePrint(3, "variable-print", {
        declareX,
        printVariableInstruction("x"),
        printVariableInstruction("y")
    });
    variablePrint.executeNext(0, 1, 0);
    variablePrint.executeNext(0, 2, 0);
    variablePrint.executeNext(0, 3, 0);
    expect(variablePrint.isFinished(), "variable PRINT instructions must execute and finish");
    const auto variableLogs = variablePrint.logs();
    expect(variableLogs.size() == 2, "PRINT must log both declared and auto-declared variables");
    expect(variableLogs[0].message == "Value from: 42",
           "PRINT must append the declared variable value");
    expect(variableLogs[1].message == "Value from: 0",
           "undeclared variables used by PRINT must default to zero");

    Instruction sleep;
    sleep.type = InstructionType::SLEEP;
    sleep.sleepTicks = 5;
    Process sleeper(4, "sleeper", {sleep, printInstruction()});
    sleeper.executeNext(0, 1, 2);
    expect(sleeper.isSleeping(5), "SLEEP must retain its wake boundary");
    sleeper.executeNext(1, 5, 2);
    expect(!sleeper.isSleeping(6), "the process must wake after five ticks");
    sleeper.executeNext(1, 6, 2);
    sleeper.executeNext(2, 7, 2);
    expect(sleeper.currentLine() == 1, "sleeping must not consume busy-wait cycles");
    sleeper.executeNext(3, 8, 2);
    expect(sleeper.isFinished(), "a waking process must resume after its instruction delay");

    Process maxDelay(5, "max-delay", std::vector<Instruction>(2, printInstruction()));
    maxDelay.executeNext(0, 1, 4294967296ULL);
    maxDelay.executeNext(1, 2, 4294967296ULL);
    expect(maxDelay.currentLine() == 1, "the supported 2^32 delay must not wrap to zero");

    std::cout << "PASS: per-process instruction delays, core changes, zero delay, SLEEP, and 2^32 delay\n";
}
