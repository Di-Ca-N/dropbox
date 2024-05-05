cmake_minimum_required(VERSION 3.27)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

project("Dropbox")

find_package(Threads REQUIRED)

add_executable(server
    server/main.cpp
)
add_executable(client
    client/main.cpp
    client/CLI.cpp
    client/Connection.cpp
    client/ClientMonitor.cpp
    client/ServerMonitor.cpp
)

target_include_directories(server PUBLIC common)
target_include_directories(client PUBLIC common)

target_link_libraries(client ${CMAKE_THREAD_LIBSS_INIT})

