cmake_minimum_required(VERSION 3.27)
project("Dropbox")

add_executable(server
    server/main.cpp
)
add_executable(client
    client/main.cpp
)

target_include_directories(server PUBLIC common)
target_include_directories(client PUBLIC common)