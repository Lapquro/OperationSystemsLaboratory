cmake_minimum_required(VERSION 4.2)
project(OS_Lab3 C)

set(CMAKE_C_STANDARD 23)

find_package(Threads REQUIRED)

add_executable(server server.c)
target_compile_options(server PRIVATE -Wall -Wextra)
target_link_libraries(server Threads::Threads rt)

add_executable(client client.c)
target_compile_options(client PRIVATE -Wall -Wextra)
target_link_libraries(client Threads::Threads rt)
