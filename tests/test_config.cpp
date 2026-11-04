#include "Config.h"
#include <fstream>
#include <iostream>
static void t(const char* label, const std::string& content) {
    { std::ofstream f("c.txt", std::ios::binary); f << content; }
    Config c; std::string e;
    bool ok = loadConfig("c.txt", c, e);
    std::cout << (ok ? "OK   " : "FAIL ") << label << " -> " << (ok ? "cpu=" + std::to_string(c.numCpu) + " sch=" + c.scheduler + " q=" + std::to_string(c.quantumCycles) + " d=" + std::to_string(c.delaysPerExec) : e) << "\n";
}
int main() {
    t("default sample", "num-cpu 4\nscheduler \"rr\"\nquantum-cycles 5\nbatch-process-freq 1\nmin-ins 1000\nmax-ins 2000\ndelay-per-exec 0");
    t("crlf+bom+spec spelling", "\xEF\xBB\xBFnum-cpu 8\r\nscheduler \"FCFS\"\r\ndelays-per-exec 3\r\n");
    t("comments/blank", "# hi\n\nnum-cpu 2\n");
    t("cpu 0", "num-cpu 0");
    t("cpu 129", "num-cpu 129");
    t("cpu 128", "num-cpu 128");
    t("cpu negative", "num-cpu -4");
    t("cpu junk", "num-cpu 4abc");
    t("cpu float", "num-cpu 2.5");
    t("cpu huge", "num-cpu 99999999999999999999999");
    t("bad sched", "scheduler sjf");
    t("quantum 0", "quantum-cycles 0");
    t("quantum 2^32 ok", "quantum-cycles 4294967296");
    t("quantum 2^32+1", "quantum-cycles 4294967297");
    t("min>max", "min-ins 500\nmax-ins 100");
    t("min 0", "min-ins 0");
    t("delay 0 ok", "delay-per-exec 0");
    t("delay neg", "delay-per-exec -1");
    t("unknown key", "num-cpu 4\nquantum-cycle 5");
    t("missing value", "num-cpu");
    t("extra text", "num-cpu 4 5");
    t("batch 0", "batch-process-freq 0");
    std::string e; Config c; c.minIns=5; c.maxIns=2;
    std::cout << "validate(min>max): " << validateConfig(c,e) << " " << e << "\n";
    Config d; std::cout << "missing file: " << loadConfig("nope.txt", d, e) << " " << e << "\n";
}
