CSOPESY Major Output 1 - Process Scheduler and CLI
===================================================
Group members:
  1. Chiu, Kristopher Lance
  2. Ramirez, Diana Angela
  3. Yamsuan, Rhian Claire
  4. Ke, Xan Luo

Entry file (main function): src/main.cpp

BUILD (CMake, works on Windows/Linux/macOS)
  Run these commands from the project root (the folder containing config.txt):
  cmake -S . -B build
  cmake --build build
  ./csopesy          (Windows PowerShell: .\csopesy.exe)

BUILD (without CMake)
  g++ -std=c++17 -pthread src/*.cpp -o csopesy

RUN
  Type "initialize" first (reads config.txt), then: screen -s <name>, screen -r <name>,
  screen -ls, scheduler-start, scheduler-stop, report-util, exit.

CONFIG FILE (config.txt, one "<parameter> <value>" per line, read by "initialize")
  num-cpu             [1, 128]
  scheduler           "fcfs" or "rr"
  quantum-cycles      [1, 2^32]   (rr only)
  batch-process-freq  [1, 2^32]   (scheduler-start: new process every N CPU cycles)
  min-ins / max-ins   [1, 2^32]   (min-ins must be <= max-ins)
  delay-per-exec      [0, 2^32]   (busy-wait cycles between instructions; "delays-per-exec" also accepted)
  Rules: lines starting with # and blank lines are ignored; keys missing from the file use the
  defaults; unknown keys, non-numeric / negative values and out-of-range values make "initialize"
  fail with a message naming the line (the emulator stays uninitialized).

PROCESS NAMES
  Names are case-sensitive and unique for the whole session (running OR finished).
  scheduler-start names processes p01, p02, ... and skips names already used by "screen -s".