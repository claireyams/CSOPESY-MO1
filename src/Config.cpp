#include "Config.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <limits>

#include "Utils.h"

namespace {

// The spec writes every upper bound as 2^32 (inclusive).
constexpr std::uint64_t kMax32 = 4294967296ULL;  // 2^32
constexpr std::uint64_t kUnparsable = std::numeric_limits<std::uint64_t>::max();

std::string num(std::uint64_t v) { return std::to_string(v); }

// Strict unsigned parse: digits only (no sign, no decimals, no "5abc").
// Values too large for uint64 saturate so the range check reports them as out of range.
bool parseUnsigned(const std::string& s, std::uint64_t& out) {
    if (s.empty()) return false;
    std::uint64_t v = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
        const std::uint64_t d = static_cast<std::uint64_t>(c - '0');
        if (v > (kUnparsable - d) / 10) { out = kUnparsable; return true; }  // overflow -> saturate
        v = v * 10 + d;
    }
    out = v;
    return true;
}

bool inRange(const std::string& key, std::uint64_t v, std::uint64_t lo, std::uint64_t hi,
             std::string& err) {
    if (v >= lo && v <= hi) return true;
    err = "'" + key + "' must be between " + num(lo) + " and " + num(hi) + " (got " +
          (v == kUnparsable ? std::string("a value that is too large") : num(v)) + ").";
    return false;
}

// Reads `val` as a whole number for `key` and checks it against [lo, hi].
bool readNumber(const std::string& key, const std::string& val, std::uint64_t lo,
                std::uint64_t hi, std::uint64_t& out, std::string& err) {
    std::uint64_t v = 0;
    if (!parseUnsigned(val, v)) {
        err = "Invalid value for '" + key + "': '" + val + "' (expected a whole number between " +
              num(lo) + " and " + num(hi) + ").";
        return false;
    }
    if (!inRange(key, v, lo, hi, err)) return false;
    out = v;
    return true;
}

}  // namespace

bool loadConfig(const std::string& path, Config& out, std::string& err) {
    std::ifstream in(path);
    if (!in) {
        err = "Cannot open '" + path + "'. Make sure it is in the folder you ran the program from.";
        return false;
    }

    Config cfg;  // starts with defaults; keys missing from the file keep their default value
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        // Windows editors (Notepad) may prepend a UTF-8 byte-order mark to the first line.
        if (lineNo == 1 && line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line.erase(0, 3);

        auto words = splitWords(line);
        if (words.empty() || words[0][0] == '#') continue;  // blank line or comment

        const std::string where = "config.txt line " + std::to_string(lineNo) + ": ";
        const std::string key = toLower(words[0]);

        if (words.size() < 2) {
            err = where + "missing value for '" + key + "'.";
            return false;
        }
        if (words.size() > 2) {
            err = where + "expected '<parameter> <value>' but found extra text after '" +
                  words[1] + "'.";
            return false;
        }

        std::string val = words[1];
        val.erase(std::remove(val.begin(), val.end(), '"'), val.end());  // scheduler "rr"

        std::string e;
        std::uint64_t n = 0;
        bool ok = true;

        if (key == "num-cpu") {
            ok = readNumber(key, val, 1, 128, n, e);
            if (ok) cfg.numCpu = static_cast<int>(n);
        } else if (key == "scheduler") {
            cfg.scheduler = toLower(val);  // value itself is checked in validateConfig()
        } else if (key == "quantum-cycles") {
            ok = readNumber(key, val, 1, kMax32, n, e);
            if (ok) cfg.quantumCycles = n;
        } else if (key == "batch-process-freq") {
            ok = readNumber(key, val, 1, kMax32, n, e);
            if (ok) cfg.batchProcessFreq = n;
        } else if (key == "min-ins") {
            ok = readNumber(key, val, 1, kMax32, n, e);
            if (ok) cfg.minIns = n;
        } else if (key == "max-ins") {
            ok = readNumber(key, val, 1, kMax32, n, e);
            if (ok) cfg.maxIns = n;
        } else if (key == "delay-per-exec" || key == "delays-per-exec") {
            // The spec table says "delays-per-exec" but its sample file says "delay-per-exec";
            // accept both spellings.
            ok = readNumber("delay-per-exec", val, 0, kMax32, n, e);
            if (ok) cfg.delaysPerExec = n;
        } else {
            // Unknown key: fail loudly. Silently ignoring a typo (e.g. "quantum-cycle") would
            // make the emulator run with a default the user never intended.
            err = where + "unknown parameter '" + words[0] +
                  "'. Valid parameters: num-cpu, scheduler, quantum-cycles, batch-process-freq, "
                  "min-ins, max-ins, delay-per-exec.";
            return false;
        }

        if (!ok) {
            err = where + e;
            return false;
        }
    }

    if (!validateConfig(cfg, err)) return false;
    out = cfg;
    return true;
}

bool validateConfig(const Config& cfg, std::string& err) {
    if (cfg.numCpu < 1 || cfg.numCpu > 128) {
        err = "'num-cpu' must be between 1 and 128 (got " + std::to_string(cfg.numCpu) + ").";
        return false;
    }
    if (cfg.scheduler != "fcfs" && cfg.scheduler != "rr") {
        err = "'scheduler' must be \"fcfs\" or \"rr\" (got \"" + cfg.scheduler + "\").";
        return false;
    }
    if (!inRange("quantum-cycles", cfg.quantumCycles, 1, kMax32, err)) return false;
    if (!inRange("batch-process-freq", cfg.batchProcessFreq, 1, kMax32, err)) return false;
    if (!inRange("min-ins", cfg.minIns, 1, kMax32, err)) return false;
    if (!inRange("max-ins", cfg.maxIns, 1, kMax32, err)) return false;
    if (cfg.minIns > cfg.maxIns) {
        err = "'min-ins' (" + num(cfg.minIns) + ") must not be greater than 'max-ins' (" +
              num(cfg.maxIns) + ").";
        return false;
    }
    if (!inRange("delay-per-exec", cfg.delaysPerExec, 0, kMax32, err)) return false;
    return true;
}
