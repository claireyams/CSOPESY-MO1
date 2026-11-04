CSOPESY Major Output 1 - Process Scheduler and CLI
===================================================
Group members:
  1. Chiu, Kristopher Lance
  2. Ramirez, Diana Angela
  3. Yamsuan, Rhian Claire
  4. Ke, Xan Luo

Entry file (main function): src/main.cpp

BUILD (CMake, works on Windows/Linux/macOS)
  mkdir build && cd build
  cmake ..
  cmake --build .
  ./csopesy          (Windows: csopesy.exe)   <- run from the folder that contains config.txt

BUILD (without CMake)
  g++ -std=c++17 -pthread src/*.cpp -o csopesy

RUN
  Type "initialize" first (reads config.txt), then: screen -s <name>, screen -r <name>,
  screen -ls, scheduler-start, scheduler-stop, report-util, exit.

WORK DIVISION
  Member 1 - CLI / Console        : src/Console.cpp
      banner + developer names, screen -s / -r / -ls parsing, process screen loop,
      process-smi output, clearing the screen, error messages, report-util wiring.
  Member 2 - Scheduler            : src/Scheduler.cpp
      core worker logic: FCFS + Round Robin (quantum), preemption, sleeping processes
      relinquishing the core, delay-per-exec busy-wait, finished list, tick barrier.
  Member 3 - Processes/Instructions: src/Process.cpp, src/Instruction.cpp
      PRINT/DECLARE/ADD/SUBTRACT/SLEEP/FOR semantics, uint16 clamping, random program
      generator (FOR nesting <= 3), flattenProgram().
  Member 4 - Config/Report/Docs   : src/Config.cpp, src/Report.cpp, src/ProcessFactory.cpp
      config validation + error messages, screen -ls / report-util formatting and
      csopesy-log.txt, duplicate-name checks, README, PPT technical report, test configs,
      quiz-day demo videos.

Search the code for "TODO(Member N)" to find each person's tasks.

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

TESTS: see tests/README.txt (sample configs + unit tests, no recompiling needed for configs).


PPT TECHNICAL REPORT SECTIONS (required)
  Command recognition | Console UI implementation | Command interpreter implementation
  Process representation | Scheduler implementation
