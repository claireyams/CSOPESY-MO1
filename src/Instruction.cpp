#include "Instruction.h"

#include <random>

std::vector<Instruction> generateRandomProgram(std::uint64_t minIns,
                                               std::uint64_t maxIns,
                                               const std::string& processName) {
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<std::uint64_t> lenDist(minIns, maxIns);
    const std::uint64_t count = lenDist(rng);

    // PLACEHOLDER: every instruction is a PRINT so the project runs end-to-end.
    // TODO(Member 3): randomize across PRINT / DECLARE / ADD / SUBTRACT / SLEEP / FOR,
    //   - FOR bodies can nest up to 3 levels,
    //   - make sure the *executed* instruction count (after unrolling FOR) lands in [min, max],
    //   - SLEEP takes a uint8 number of ticks,
    //   - PRINT msg = "Hello world from <processName>!" (optionally with one variable).
    std::vector<Instruction> program;
    program.reserve(count);
    for (std::uint64_t i = 0; i < count; ++i) {
        Instruction ins;
        ins.type = InstructionType::PRINT;
        ins.msg = "Hello world from " + processName + "!";
        program.push_back(std::move(ins));
    }
    return program;
}

std::vector<Instruction> flattenProgram(const std::vector<Instruction>& nested) {
    // TODO(Member 3): recursively unroll FOR instructions (body repeated `repeats` times).
    // Placeholder: assumes there are no FOR instructions.
    return nested;
}
