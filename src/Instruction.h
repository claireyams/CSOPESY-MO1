#pragma once
#include <cstdint>
#include <string>
#include <vector>

// barebones process instructions
enum class InstructionType { PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR };

struct Operand {
    bool          isVariable = false;
    std::string   varName;
    std::uint16_t literal = 0;
};

struct Instruction {
    InstructionType type = InstructionType::PRINT;

    // PRINT("msg" + var)
    std::string msg;
    std::string printVar;          // empty if the PRINT has no variable

    // DECLARE(target, lhs.literal) | ADD/SUBTRACT(target, lhs, rhs)
    std::string target;
    Operand     lhs;
    Operand     rhs;

    // SLEEP(X)
    std::uint8_t sleepTicks = 0;

    // FOR([body], repeats) - nesting up to 3 levels
    std::vector<Instruction> body;
    std::uint32_t            repeats = 0;
};

// builds a random (possibly nested) program whose executed length is within [minIns, maxIns].
// PRINT msgs default to "Hello world from <processName>!".
std::vector<Instruction> generateRandomProgram(std::uint64_t minIns,
                                               std::uint64_t maxIns,
                                               const std::string& processName);

// expands FOR loops into a flat list so Process can just walk it with a program counter
// (this also makes "current instruction line / lines of code" trivial).
std::vector<Instruction> flattenProgram(const std::vector<Instruction>& nested);
