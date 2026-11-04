#include "ProcessFactory.h"

#include <cstdio>

#include "Instruction.h"

bool ProcessFactory::reserveName(const std::string& name) {
    std::lock_guard<std::mutex> lk(namesMtx_);
    return usedNames_.insert(name).second;
}

bool ProcessFactory::isNameTaken(const std::string& name) const {
    std::lock_guard<std::mutex> lk(namesMtx_);
    return usedNames_.count(name) > 0;
}

// Name is already reserved by the caller. The (potentially large) program is generated
// outside the lock so the tick thread and console thread don't block each other.
std::shared_ptr<Process> ProcessFactory::build(const std::string& name) {
    const int id = nextId_++;
    auto program = flattenProgram(generateRandomProgram(cfg_.minIns, cfg_.maxIns, name));
    return std::make_shared<Process>(id, name, std::move(program));
}

std::shared_ptr<Process> ProcessFactory::createNamed(const std::string& name) {
    if (name.empty() || !reserveName(name)) return nullptr;
    return build(name);
}

std::shared_ptr<Process> ProcessFactory::createAuto() {
    // Pick the next unused pNN and reserve it in one critical section, so two callers can
    // never get the same name and a user-made "p05" is simply skipped.
    std::string name;
    {
        std::lock_guard<std::mutex> lk(namesMtx_);
        char buf[24];
        do {
            std::snprintf(buf, sizeof(buf), "p%02d", autoCounter_++);
        } while (usedNames_.count(buf) > 0);
        name = buf;
        usedNames_.insert(name);
    }
    return build(name);
}
