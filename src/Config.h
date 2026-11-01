#pragma once
#include <cstdint>
#include <string>

// Mirrors config.txt (see "The config.txt file and initialize command" in the spec).
struct Config {
    int           numCpu           = 4;        // [1, 128]
    std::string   scheduler        = "rr";     // "fcfs" or "rr"
    std::uint64_t quantumCycles    = 5;        // [1, 2^32]  (rr only)
    std::uint64_t batchProcessFreq = 1;        // [1, 2^32]  (scheduler-start)
    std::uint64_t minIns           = 1000;     // [1, 2^32]
    std::uint64_t maxIns           = 2000;     // [1, 2^32]
    std::uint64_t delaysPerExec    = 0;        // [0, 2^32]  busy-waiting delay
};

// Reads a space-separated config file. Returns false and fills `err` on failure.
bool loadConfig(const std::string& path, Config& out, std::string& err);

// Range / sanity checks (e.g. min-ins <= max-ins, scheduler is fcfs|rr, ...).
bool validateConfig(const Config& cfg, std::string& err);
