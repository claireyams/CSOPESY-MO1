#pragma once
#include <atomic>
#include <memory>
#include <string>

#include "Config.h"
#include "Process.h"

// Creates processes with generated instruction lists.
class ProcessFactory {
public:
    explicit ProcessFactory(const Config& cfg) : cfg_(cfg) {}

    // For "screen -s <name>"
    std::shared_ptr<Process> createNamed(const std::string& name);

    // For "scheduler-start": human-readable names p01, p02, ..., p1240
    std::shared_ptr<Process> createAuto();

private:
    Config           cfg_;
    std::atomic<int> nextId_{1};
    std::atomic<int> autoCounter_{1};
};
