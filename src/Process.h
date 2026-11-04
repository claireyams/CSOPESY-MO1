#pragma once
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "Instruction.h"

struct LogEntry {
    std::string timestamp;  
    int         core = -1;
    std::string message;    
};

// a single emulated process. Worker (core) threads call executeNext();
// the console thread reads state for process-smi / screen -ls
class Process {
public:
    Process(int id, std::string name, std::vector<Instruction> flatProgram);

    // called by the scheduler
    // Runs one CPU cycle on 'coreId' at CPU tick 'currentTick'.
    // Busy-wait cycles consume this process's remaining instruction delay.
    // Does nothing if the process is finished or still sleeping.
    void executeNext(int coreId, std::uint64_t currentTick,
                     std::uint64_t delaysPerExec = 0);

    // read-only info for the console / reports 
    int                      id() const { return id_; }
    const std::string&       name() const { return name_; }
    const std::string&       createdAt() const { return createdAt_; }
    std::size_t              currentLine() const;
    std::size_t              totalLines() const { return totalLines_; }
    bool                     isFinished() const;
    bool                     isSleeping(std::uint64_t currentTick) const;
    int                      lastCore() const;
    std::vector<LogEntry>    logs() const; 

private:
    std::uint16_t resolve(const Operand& op);
    static std::uint16_t clampAdd(std::uint32_t a, std::uint32_t b);
    static std::uint16_t clampSub(std::int32_t a, std::int32_t b);

    void markFinished();

    const int         id_;
    const std::string name_;
    const std::string createdAt_;

    std::vector<Instruction>                      program_;
    const std::size_t                             totalLines_;
    std::size_t                                   pc_ = 0;
    std::unordered_map<std::string, std::uint16_t> vars_;
    std::vector<LogEntry>                         logs_;
    std::uint64_t                                 sleepUntilTick_ = 0;
    // follows the process across RR preemption and moves between cores.
    std::uint64_t                                 delayLeft_ = 0;
    bool                                          finished_ = false;
    int                                           lastCore_ = -1;

    mutable std::mutex mtx_;
};
