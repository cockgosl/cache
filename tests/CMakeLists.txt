find_package(GTest REQUIRED)

set (TEST_SOURCES
    lru_tests.cpp
    lfu_tests.cpp
    arc_tests.cpp
    lirs_tests.cpp
    2q_tests.cpp
    multi_cache_tests.cpp
)

add_executable(cache_tests ${TEST_SOURCES})

target_link_libraries(cache_tests
    PRIVATE
        cache
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cache_tests)
