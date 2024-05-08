cmake_minimum_required(VERSION 3.27)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip 
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    #find_package(GTest REQUIRED)
    #include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(tests
        test/hello_test.cpp
    )

    target_link_libraries(tests GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(tests)
endif(BUILD_TESTS)

