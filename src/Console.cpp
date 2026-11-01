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
              << "  TODO(Member 1): put your group members' names here\n\n"
              << "Last updated: TODO\n"
              << "-------------------------------------------\n";
}

void Console::run() {
    clearScreen();
    printBanner();

    std::string line;
    while (true) {
        std::cout << "root:\\> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (!handleMainCommand(line)) break;
    }
    // Scheduler threads are joined in ~Scheduler() when scheduler_ is destroyed.
}

bool Console::handleMainCommand(const std::string& line) {
    const auto args = splitWords(line);
    if (args.empty()) return true;

    const std::string cmd = toLower(args[0]);

    if (cmd == "exit") return false;

    // Spec: nothing but "initialize" and "exit" is recognized before initialization.
    if (!initialized_) {
        if (cmd == "initialize") cmdInitialize();
        else std::cout << "Please run 'initialize' first.\n";
        return true;
    }

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
    // TODO(Member 1): print feedback, and avoid doing anything weird if already running.
    scheduler_->setGenerating(true);
}

void Console::cmdSchedulerStop() {
    scheduler_->setGenerating(false);
}

void Console::cmdReportUtil() {
    // TODO(Member 1 + 4): build the report, save to "csopesy-log.txt", then print
    //   "Report generated at <path>!" (see mockup, lines 56-57 of the spec).
    //   Hint: saveReport("csopesy-log.txt", buildUtilizationReport(scheduler_->snapshot()));
}

void Console::cmdScreen(const std::vector<std::string>& args) {
    // TODO(Member 1): parse and dispatch:
    //   screen -s <name>  -> cmdScreenNew(name)
    //   screen -r <name>  -> cmdScreenResume(name)
    //   screen -ls        -> cmdScreenList()
    //   anything else     -> print a usage message
    (void)args;
    std::cout << "screen: not implemented yet.\n";
}

void Console::cmdScreenList() {
    // TODO(Member 1): print buildUtilizationReport(scheduler_->snapshot()).
}

void Console::cmdScreenNew(const std::string& name) {
    // TODO(Member 1): create via factory_->createNamed(name), scheduler_->addProcess(...),
    //   then enterProcessScreen(proc). Reject duplicate / empty names.
    (void)name;
}

void Console::cmdScreenResume(const std::string& name) {
    // TODO(Member 1): auto p = scheduler_->findActiveProcess(name);
    //   if (!p) print "Process <name> not found."; else enterProcessScreen(p).
    (void)name;
}

void Console::enterProcessScreen(std::shared_ptr<Process> proc) {
    // TODO(Member 1): clearScreen(), print process info, then loop:
    //   "process-smi" -> printProcessSmi(*proc)
    //   "exit"        -> clearScreen(), reprint banner, return to main menu
    (void)proc;
}

void Console::printProcessSmi(const Process& proc) const {
    // TODO(Member 1): print (see spec mockup lines 162-198):
    //   Process name: <name>
    //   ID: <id>
    //   Logs:
    //   (<timestamp>) Core:<n> "<message>"   <- one per proc.logs()
    //   (blank)
    //   Current instruction line: <currentLine>
    //   Lines of code: <totalLines>
    //   If proc.isFinished() print "Finished!" after the logs.
    (void)proc;
}
