cmake_minimum_required(VERSION 3.16)
project(SPO_lab3 C)

set(CMAKE_C_STANDARD 99)
set(EXECUTABLE_OUTPUT_PATH build CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH build CACHE PATH "Build directory" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)
set(CMAKE_BUILD_FILES_DIRECTORY build)
set(CMAKE_BUILD_DIRECTORY build)
SET(EXECUTABLE_OUTPUT_PATH build)
SET(LIBRARY_OUTPUT_PATH build lib)

add_executable(lab3 main.c server/server.c client/client.c coop/coop.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
target_link_libraries(lab3 PRIVATE ncurses Threads::Threads)