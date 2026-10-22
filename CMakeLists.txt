cmake_minimum_required(VERSION 3.16)

project(cache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Common compiler warnings
add_library(cache INTERFACE)
target_compile_options(cache INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra>
)
target_include_directories(cache INTERFACE include)

# Main application
add_executable(cache_app
    src/main.cpp
)

target_link_libraries(cache_app PRIVATE cache)

include(CTest)
# Google Test
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

option (BENCHMARKS "build benchmarks" OFF)
# Benchmark
if (BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
