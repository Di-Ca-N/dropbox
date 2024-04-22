cmake_minimum_required(VERSION 3.27)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

project("Dropbox")

add_executable(server
    server/main.cpp
)
add_executable(client
    client/main.cpp
)

target_include_directories(server PUBLIC common)
target_include_directories(client PUBLIC common)