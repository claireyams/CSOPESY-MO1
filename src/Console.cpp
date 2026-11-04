#include "Console.h"

#include <iostream>

#include "Report.h"
#include "Utils.h"

void Console::printBanner() const {
    std::cout << R"BANNER(
  ____ ____   ___  ____  _____ ____ __   __
 / ___/ ___| / _ \|  _ \| ____/ ___|\ \ / /
| |   \___ \| | | | |_) |  _| \___ \ \ V /
| |___ ___) | |_| |  __/| |___ ___) | | |
 \____|____/ \___/|_|   |_____|____/  |_|
)BANNER";
    std::cout << "-------------------------------------------\n"
              << "Welcome to CSOPESY Emulator!\n\n"
              << "Developers:\n"
              << "  Kristopher Lance Chiu\n"
              << "  Diana Angela Ramirez\n"
              << "  Rhian Claire Yamsuan\n"
              << "  Xan Luo Ke\n\n"
              << "Last updated: 11-05-2026\n"
              << "-------------------------------------------\n";
}

void Console::run() {
    clearScreen();
    printBanner();

    std::string line;
    while (true) {
        std::cout << "\nroot:\\> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (!handleMainCommand(line)) break;
    }
    // Scheduler threads are joined in ~Scheduler() when scheduler_ is destroyed.
}

bool Console::handleMainCommand(const std::string& line) {
    const auto args = splitWords(line);
    if (args.empty()) return true;

    const std::string cmd = toLower(args[0]);

    // Spec: nothing but "initialize" and "exit" is recognized before initialization.
    if (!initialized_ && cmd != "initialize" && cmd != "exit") {
        std::cout << "Please run 'initialize' first.\n";
        return true;
    }

    const bool takesNoArguments = cmd == "initialize" || cmd == "exit"
        || cmd == "scheduler-start" || cmd == "scheduler-stop" || cmd == "report-util";
    if (takesNoArguments && args.size() != 1) {
        std::cout << "Usage: " << cmd << "\n";
        return true;
    }

    if (cmd == "exit") return false;

    if (cmd == "initialize")           cmdInitialize();      // re-initialize
    else if (cmd == "screen")          cmdScreen(args);
    else if (cmd == "scheduler-start") cmdSchedulerStart();
    else if (cmd == "scheduler-stop")  cmdSchedulerStop();
    else if (cmd == "report-util")     cmdReportUtil();
    else std::cout << "Unknown command: " << args[0] << "\n";
    return true;
}

void Console::cmdInitialize() {
    // Tear down any previous run (scheduler first - it references the factory).
    scheduler_.reset();
    factory_.reset();
    initialized_ = false;

    Config cfg;
    std::string err;
    if (!loadConfig("config.txt", cfg, err)) {
        std::cout << "Initialization failed: " << err << "\n";
        return;
    }
    config_ = cfg;
    factory_ = std::make_unique<ProcessFactory>(config_);
    scheduler_ = std::make_unique<Scheduler>(config_, *factory_);
    scheduler_->start();
    initialized_ = true;

    std::cout << "Initialized: " << config_.numCpu << " CPU(s), scheduler="
              << config_.scheduler << "\n";
}

void Console::cmdSchedulerStart() {
    if (scheduler_->isGenerating()) {
        std::cout << "Process generation is already running.\n";
        return;
    }
    scheduler_->setGenerating(true);
    std::cout << "Process generation started.\n";
}

void Console::cmdSchedulerStop() {
    if (!scheduler_->isGenerating()) {
        std::cout << "Process generation is already stopped.\n";
        return;
    }
    scheduler_->setGenerating(false);
    std::cout << "Process generation stopped.\n";
}

void Console::cmdReportUtil() {
    const std::string path = "csopesy-log.txt";
    const auto report = buildUtilizationReport(scheduler_->snapshot());
    if (saveReport(path, report))
        std::cout << "Report generated at " << path << "!\n";
    else
        std::cout << "Could not save report to " << path << ".\n";
}

void Console::cmdScreen(const std::vector<std::string>& args) {
    if (args.size() == 2 && toLower(args[1]) == "-ls")
        cmdScreenList();
    else if (args.size() == 3 && toLower(args[1]) == "-s")
        cmdScreenNew(args[2]);
    else if (args.size() == 3 && toLower(args[1]) == "-r")
        cmdScreenResume(args[2]);
    else
        std::cout << "Usage: screen -s <name> | screen -r <name> | screen -ls\n";
}

void Console::cmdScreenList() {
    std::cout << buildUtilizationReport(scheduler_->snapshot());
}

void Console::cmdScreenNew(const std::string& name) {
    if (name.empty()) {
        std::cout << "Process name cannot be empty.\n";
        return;
    }
    // The factory remembers every name ever used (running or finished), so reports never
    // show two different processes with the same name.
    if (factory_->isNameTaken(name)) {
        std::cout << "Process " << name << " already exists.\n";
        return;
    }

    auto proc = factory_->createNamed(name);
    if (!proc) {
        std::cout << "Could not create process " << name << ".\n";
        return;
    }
    scheduler_->addProcess(proc);
    enterProcessScreen(proc);
}

void Console::cmdScreenResume(const std::string& name) {
    auto proc = scheduler_->findActiveProcess(name);
    if (!proc) {
        std::cout << "Process " << name << " not found.\n";
        return;
    }
    enterProcessScreen(proc);
}

void Console::enterProcessScreen(std::shared_ptr<Process> proc) {
    clearScreen();
    printProcessSmi(*proc);

    std::string line;
    while (true) {
        std::cout << "\nroot:\\> " << std::flush;
        if (!std::getline(std::cin, line)) return;
        const auto args = splitWords(line);
        if (args.empty()) continue;

        const std::string cmd = toLower(args[0]);
        if (args.size() == 1 && cmd == "exit") {
            clearScreen();
            printBanner();
            return;
        }
        if (args.size() == 1 && cmd == "process-smi")
            printProcessSmi(*proc);
        else
            std::cout << "Available process commands: process-smi, exit\n";
    }
}

void Console::printProcessSmi(const Process& proc) const {
    const bool finished = proc.isFinished();
    std::cout << "Process name: " << proc.name() << "\n"
              << "ID: " << proc.id() << "\n"
              << "Logs:\n";
    for (const auto& log : proc.logs())
        std::cout << "(" << log.timestamp << ") Core:" << log.core
                  << " \"" << log.message << "\"\n";

    if (finished)
        std::cout << "\nFinished!\n\n";
    else
        std::cout << "\nCurrent instruction line: " << proc.currentLine() << "\n"
                  << "Lines of code: " << proc.totalLines() << "\n\n";
}
