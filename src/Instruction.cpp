#include "Instruction.h"

#include <algorithm>
#include <random>

namespace {

constexpr int           kMaxForDepth   = 3;    // for loops can be nested up to 3 times
constexpr int           kForChancePct  = 10;   // chance that the next generated item is a for
constexpr std::uint64_t kMaxRepeats    = 5;
constexpr std::uint64_t kMaxBodyLen    = 25;   // instructions inside one for body (before repeats)
constexpr std::uint64_t kMaxSleepTicks = 5;
constexpr std::uint64_t kMaxLiteral    = 1000; // literal operands for ADD / SUBTRACT

const char* const kVarNames[] = {"x", "y", "z", "a", "b"};

std::mt19937_64& rng() {
    static thread_local std::mt19937_64 r{std::random_device{}()};
    return r;
}

std::uint64_t randInt(std::uint64_t lo, std::uint64_t hi) {
    std::uniform_int_distribution<std::uint64_t> d(lo, hi);
    return d(rng());
}

std::string randVar() { return kVarNames[randInt(0, std::size(kVarNames) - 1)]; }

Operand randOperand() {
    Operand o;
    if (randInt(0, 1) == 0) {
        o.isVariable = true;
        o.varName = randVar();
    } else {
        o.literal = static_cast<std::uint16_t>(randInt(0, kMaxLiteral));
    }
    return o;
}

// One non-FOR instruction (always counts as exactly 1 executed line).
Instruction makeSimple(const std::string& processName) {
    Instruction ins;
    const auto roll = randInt(0, 99);
    if (roll < 30) {
        ins.type = InstructionType::PRINT;
        // msg is always "Hello world from <process_name>!" unless a test case says otherwise
        ins.msg = "Hello world from " + processName + "!";
    } else if (roll < 50) {
        ins.type = InstructionType::DECLARE;
        ins.target = randVar();
        ins.lhs.literal = static_cast<std::uint16_t>(randInt(0, 65535));
    } else if (roll < 70) {
        ins.type = InstructionType::ADD;
        ins.target = randVar();
        ins.lhs = randOperand();
        ins.rhs = randOperand();
    } else if (roll < 90) {
        ins.type = InstructionType::SUBTRACT;
        ins.target = randVar();
        ins.lhs = randOperand();
        ins.rhs = randOperand();
    } else {
        ins.type = InstructionType::SLEEP;
        ins.sleepTicks = static_cast<std::uint8_t>(randInt(1, kMaxSleepTicks));
    }
    return ins;
}

// appends instructions to 'out' whose executed length (FORs unrolled) is exactly 'budget'
void build(std::uint64_t budget, int depth, const std::string& processName,
           std::vector<Instruction>& out) {
    while (budget > 0) {
        const bool canLoop = depth < kMaxForDepth && budget >= 2;
        if (canLoop && randInt(0, 99) < kForChancePct) {
            const std::uint64_t repeats = randInt(2, std::min(kMaxRepeats, budget));
            const std::uint64_t bodyLen = randInt(1, std::min(kMaxBodyLen, budget / repeats));

            Instruction loop;
            loop.type = InstructionType::FOR;
            loop.repeats = static_cast<std::uint32_t>(repeats);
            build(bodyLen, depth + 1, processName, loop.body);
            out.push_back(std::move(loop));
            budget -= repeats * bodyLen;
        } else {
            out.push_back(makeSimple(processName));
            --budget;
        }
    }
}

void flattenInto(const std::vector<Instruction>& src, std::vector<Instruction>& out) {
    for (const auto& ins : src) {
        if (ins.type == InstructionType::FOR) {
            std::vector<Instruction> flatBody;
            flattenInto(ins.body, flatBody);
            for (std::uint32_t r = 0; r < ins.repeats; ++r)
                out.insert(out.end(), flatBody.begin(), flatBody.end());
        } else {
            out.push_back(ins);
        }
    }
}

}  // namespace

std::vector<Instruction> generateRandomProgram(std::uint64_t minIns,
                                               std::uint64_t maxIns,
                                               const std::string& processName) {
    if (minIns > maxIns) std::swap(minIns, maxIns);
    const std::uint64_t target = randInt(minIns, maxIns);

    std::vector<Instruction> program;
    build(target, 0, processName, program);
    return program;
}

std::vector<Instruction> flattenProgram(const std::vector<Instruction>& nested) {
    std::vector<Instruction> flat;
    flattenInto(nested, flat);
    return flat;
}