cmake_minimum_required(VERSION 3.27)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

project("Dropbox")

option(BUILD_TESTS "Build unit tests" OFF)

add_executable(server
    server/main.cpp
)
add_executable(client
    client/main.cpp
)

target_include_directories(server PUBLIC common)
target_include_directories(client PUBLIC common)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(tests
        test/hello_test.cpp
    )

    target_link_libraries(tests ${GTEST_LIBRARIES} gtest_main)
endif(BUILD_TESTS)

