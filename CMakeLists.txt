cmake_minimum_required(VERSION 3.16)
project(CSOPESY_Emulator LANGUAGES CXX)

find_package(Threads REQUIRED)

add_executable(csopesy
    src/main.cpp
    src/Console.cpp
    src/Scheduler.cpp
    src/Process.cpp
    src/ProcessFactory.cpp
    src/Instruction.cpp
    src/Config.cpp
    src/Report.cpp
    src/Utils.cpp
)
target_compile_features(csopesy PRIVATE cxx_std_17)
target_link_libraries(csopesy PRIVATE Threads::Threads)
set_target_properties(csopesy PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>"
)