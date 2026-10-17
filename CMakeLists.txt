
cmake_minimum_required(VERSION 3.16)

project(cache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

# Common compiler warnings
add_library(project_options INTERFACE)

target_compile_options(project_options INTERFACE
    -Wall
    -Wextra
)

# Main application
add_executable(cache_app
    src/main.cpp
    src/cache_api.cpp
)

target_include_directories(cache_app PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(cache_app PRIVATE project_options)

# Google Test
if(BUILD_TESTING)
    find_package(GTest REQUIRED)

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/tests/*_tests.cpp"
    )

    add_executable(cache_tests ${TEST_SOURCES})

    target_include_directories(cache_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(cache_tests PRIVATE
        project_options
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(cache_tests)
endif()

# Benchmark
add_executable(cache_benchmark
    benchmarks/benchmark.cpp
)

target_include_directories(cache_benchmark PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(cache_benchmark PRIVATE project_options)

# Optional target for running benchmarks and generating plots
find_package(Python3 COMPONENTS Interpreter)

if(Python3_Interpreter_FOUND)
    add_custom_target(benchmarks
        COMMAND $<TARGET_FILE:cache_benchmark>
        COMMAND ${Python3_EXECUTABLE}
                "${PROJECT_SOURCE_DIR}/benchmarks/plot_benchmarks.py"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        DEPENDS cache_benchmark
        USES_TERMINAL
        COMMENT "Running benchmarks and generating plots"
    )
endif()
