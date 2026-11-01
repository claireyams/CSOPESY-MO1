cmake_minimum_required(VERSION 3.10)
project(csopesy_emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
target_link_libraries(csopesy Threads::Threads)

# Copy config.txt next to the executable so "initialize" can find it.
add_custom_command(TARGET csopesy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/config.txt $<TARGET_FILE_DIR:csopesy>/config.txt)
