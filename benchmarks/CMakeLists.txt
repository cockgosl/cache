add_executable(cache_benchmark benchmarks.cpp)

target_link_libraries(cache_benchmark PRIVATE cache)

# Optional target for running benchmarks and generating plots
set(Python3_EXECUTABLE "/usr/bin/python3")
find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(Python3_Interpreter_FOUND)
    add_custom_target(benchmarks
        COMMAND $<TARGET_FILE:cache_benchmark>
        COMMAND ${Python3_EXECUTABLE}
                "${CMAKE_CURRENT_SOURCE_DIR}/plot_benchmarks.py"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        DEPENDS cache_benchmark
        USES_TERMINAL
        COMMENT "Running benchmarks and generating plots"
    )
endif()
