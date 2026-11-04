TESTS (Member 4)
================
configs/NN_*.txt   valid configs for the quiz-day scenarios (copy one over ./config.txt, then run
                   "initialize"; no recompiling needed).
configs/bad_*.txt  invalid configs - "initialize" must print a clear error and stay uninitialized.

Unit tests (run from the repo root):

  g++ -std=c++17 -Isrc tests/test_config.cpp src/Config.cpp src/Utils.cpp -o test_config
  ./test_config        (writes a scratch c.txt in the current folder)

  g++ -std=c++17 -pthread -Isrc tests/test_factory.cpp src/ProcessFactory.cpp src/Process.cpp \
      src/Instruction.cpp src/Utils.cpp -o test_factory
  ./test_factory

Instruction-delay regression tests (remaining delay survives turns and core changes):

  g++ -std=c++17 -pthread -iquote src tests/test_process_delay.cpp src/Process.cpp src/Utils.cpp -o test_process_delay
  ./test_process_delay
