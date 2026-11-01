#include "ProcessFactory.h"

#include <cstdio>

#include "Instruction.h"

std::shared_ptr<Process> ProcessFactory::createNamed(const std::string& name) {
    // TODO(Member 4): reject duplicate names (coordinate with Scheduler::findActiveProcess).
    const int id = nextId_++;
    auto program = flattenProgram(generateRandomProgram(cfg_.minIns, cfg_.maxIns, name));
    return std::make_shared<Process>(id, name, std::move(program));
}

std::shared_ptr<Process> ProcessFactory::createAuto() {
    // TODO(Member 4): make sure auto names never collide with user-made names.
    char buf[24];
    std::snprintf(buf, sizeof(buf), "p%02d", autoCounter_++);
    return createNamed(buf);
}
