cmake_minimum_required(VERSION 4.2)
project(OS_Lab2 C)

set(CMAKE_C_STANDARD 23)

find_package(Threads REQUIRED)

add_executable(main main.c)
target_compile_options(main PRIVATE -Wall)
target_link_libraries(main Threads::Threads)

add_executable(test test.c)
target_compile_options(test PRIVATE -Wall)
target_link_libraries(test Threads::Threads)
