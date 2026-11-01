#include "Config.h"

#include <algorithm>
#include <fstream>

#include "Utils.h"

bool loadConfig(const std::string& path, Config& out, std::string& err) {
    std::ifstream in(path);
    if (!in) {
        err = "Cannot open '" + path + "'.";
        return false;
    }

    Config cfg;  // starts with defaults
    std::string line;
    while (std::getline(in, line)) {
        auto words = splitWords(line);
        if (words.size() < 2) continue;

        const std::string key = toLower(words[0]);
        std::string val = words[1];
        val.erase(std::remove(val.begin(), val.end(), '"'), val.end());  // scheduler "rr"

        try {
            if (key == "num-cpu")                  cfg.numCpu = std::stoi(val);
            else if (key == "scheduler")           cfg.scheduler = toLower(val);
            else if (key == "quantum-cycles")      cfg.quantumCycles = std::stoull(val);
            else if (key == "batch-process-freq")  cfg.batchProcessFreq = std::stoull(val);
            else if (key == "min-ins")             cfg.minIns = std::stoull(val);
            else if (key == "max-ins")             cfg.maxIns = std::stoull(val);
            // NOTE: the spec table says "delays-per-exec" but the sample says "delay-per-exec".
            else if (key == "delay-per-exec" || key == "delays-per-exec")
                cfg.delaysPerExec = std::stoull(val);
            else {
                // TODO(Member 4): decide what to do with unknown keys (warn? error?)
            }
        } catch (...) {
            err = "Invalid value for '" + key + "': " + val;
            return false;
        }
    }

    if (!validateConfig(cfg, err)) return false;
    out = cfg;
    return true;
}

bool validateConfig(const Config& cfg, std::string& err) {
    // TODO(Member 4): enforce every range from the spec table and produce clear messages:
    //   num-cpu [1,128], scheduler in {fcfs, rr}, quantum-cycles [1,2^32],
    //   batch-process-freq [1,2^32], min-ins/max-ins [1,2^32] and min-ins <= max-ins,
    //   delay-per-exec [0,2^32]
    (void)cfg;
    (void)err;
    return true;
}
