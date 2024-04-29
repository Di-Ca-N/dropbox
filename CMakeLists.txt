cmake_minimum_required(VERSION 3.27)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_CXX_STANDARD 20)

project("Dropbox")

add_executable(server
    server/main.cpp
    server/SyncQueue.cpp
    common/Messages.cpp
)

add_executable(client
    client/main.cpp
    common/Messages.cpp
)

target_include_directories(server PUBLIC common)
target_include_directories(client PUBLIC common)