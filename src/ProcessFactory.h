#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

#include "Config.h"
#include "Process.h"

// Creates processes with generated instruction lists.
//
// Thread-safety: createNamed() is called from the console thread and createAuto() from the
// scheduler's tick thread, so name bookkeeping is guarded by a mutex.
//
// Every name ever handed out (running OR finished) stays reserved, so csopesy-log.txt never
// lists two different processes under the same name.
class ProcessFactory {
public:
    explicit ProcessFactory(const Config& cfg) : cfg_(cfg) {}

    // For "screen -s <name>". Returns nullptr if the name is empty or already used.
    std::shared_ptr<Process> createNamed(const std::string& name);

    // For "scheduler-start": human-readable names p01, p02, ..., p1240.
    // Skips any name already taken (e.g. a user ran "screen -s p05" earlier).
    std::shared_ptr<Process> createAuto();

    // True if a process with this exact (case-sensitive) name was ever created.
    bool isNameTaken(const std::string& name) const;

private:
    bool reserveName(const std::string& name);  // false if already taken
    std::shared_ptr<Process> build(const std::string& name);

    Config           cfg_;
    std::atomic<int> nextId_{1};

    mutable std::mutex              namesMtx_;
    std::unordered_set<std::string> usedNames_;
    int                             autoCounter_ = 1;  // guarded by namesMtx_
};
