CSOPESY Major Output 1 - Process Scheduler and CLI
===================================================
Group members:
  1. <name>  (Member 1)
  2. <name>  (Member 2)
  3. Yamsuan, Rhian Claire
  4. <name>  (Member 4)

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

PPT TECHNICAL REPORT SECTIONS (required)
  Command recognition | Console UI implementation | Command interpreter implementation
  Process representation | Scheduler implementation
