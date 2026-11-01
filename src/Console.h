#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Config.h"
#include "Process.h"
#include "ProcessFactory.h"
#include "Scheduler.h"

// Main-menu console + the per-process "screen".
class Console {
public:
    Console() = default;
    void run();

private:
    // --- main menu ---------------------------------------------------------------------------
    void printBanner() const;
    bool handleMainCommand(const std::string& line);  // returns false when the user typed "exit"

    void cmdInitialize();
    void cmdScreen(const std::vector<std::string>& args);
    void cmdSchedulerStart();
    void cmdSchedulerStop();
    void cmdReportUtil();

    // --- screen command pieces ---------------------------------------------------------------
    void cmdScreenList();                                    // screen -ls
    void cmdScreenNew(const std::string& name);              // screen -s <name>
    void cmdScreenResume(const std::string& name);           // screen -r <name>
    void enterProcessScreen(std::shared_ptr<Process> proc);  // process screen loop
    void printProcessSmi(const Process& proc) const;         // process-smi

    // NOTE: declaration order matters - scheduler_ must be destroyed BEFORE factory_.
    bool                              initialized_ = false;
    Config                            config_;
    std::unique_ptr<ProcessFactory>   factory_;
    std::unique_ptr<Scheduler>        scheduler_;
};
