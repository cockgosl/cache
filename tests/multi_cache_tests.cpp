#include <gtest/gtest.h>
#include "interface/cache_api.hpp"
#include "cache_algo/lru_cache.hpp"

TEST(MultiCache, MissLoadsFromSlowMemory) {
    // Assign
    multi_cache_t<int, int> cache;

    cache.add_cache<lru_cache_t<int, int>>(2);

    // Assert
    int value;

    EXPECT_FALSE(cache.request_exclusive(1, value));

    int expected = value;

    EXPECT_TRUE(cache.request_exclusive(1, value));
    EXPECT_EQ(value, expected);
}

TEST(MultiCache, VictimPropagatesThroughLevels) {
    // Assign
    multi_cache_t<int, int> cache;

    cache.add_cache<lru_cache_t<int, int>>(1);
    cache.add_cache<lru_cache_t<int, int>>(1);

    int v1;
    int v2;
    int v3;

    // Act
    cache.request_exclusive(1, v1);
    cache.request_exclusive(2, v2);

    cache.request_exclusive(3, v3);

    // Assert
    int value;

    EXPECT_TRUE(cache.request_exclusive(2, value));
    EXPECT_EQ(value, v2);

    EXPECT_TRUE(cache.request_exclusive(1, value) == false);
    EXPECT_EQ(value, v1);
}

TEST(MultiCache, ThreeLevels) {
    // Assign
    multi_cache_t<int, int> cache;

    cache.add_cache<lru_cache_t<int, int>>(1);
    cache.add_cache<lru_cache_t<int, int>>(1);
    cache.add_cache<lru_cache_t<int, int>>(1);

    int v1;
    int v2;
    int v3;

    // Act
    cache.request_exclusive(1, v1);
    cache.request_exclusive(2, v2);
    cache.request_exclusive(3, v3);

    // L1: 3
    // L2: 2
    // L3: 1

    // Assert
    int value;

    EXPECT_TRUE(cache.request_exclusive(1, value));
    EXPECT_EQ(value, v1);
}

TEST(MultiCache, VictimMovesToSecondLevel) {
    // Assign
    multi_cache_t<int, int> cache;

    cache.add_cache<lru_cache_t<int, int>>(2);
    cache.add_cache<lru_cache_t<int, int>>(2);

    int value1;
    int value2;
    int value3;

    // Assert

    EXPECT_FALSE(cache.request_exclusive(1, value1));
    EXPECT_FALSE(cache.request_exclusive(2, value2));

    // L1 is filled. 1 is to be evicted in L2.
    EXPECT_FALSE(cache.request_exclusive(3, value3));

    // 1 is to be in L2.
    int restored;

    EXPECT_TRUE(cache.request_exclusive(1, restored));
    EXPECT_EQ(restored, value1);
}

TEST(MultiCache, WorksWithoutCaches) {
    // Assign
    multi_cache_t<int, int> cache;

    int value1;
    int value2;

    // Assert
    EXPECT_FALSE(cache.request_exclusive(1, value1));
    EXPECT_FALSE(cache.request_exclusive(1, value2));

    EXPECT_EQ(value1, value2);
}

TEST(MultiCacheInclusive, MissLoadsPageIntoAllLevels) {
    // Assign
    multi_cache_t<int, int> cache;

    cache.add_cache<lru_cache_t<int, int>>(2);
    cache.add_cache<lru_cache_t<int, int>>(3);
    cache.add_cache<lru_cache_t<int, int>>(4);

    // Assert
    int value;

    EXPECT_FALSE(cache.request_inclusive(1, value));

    int expected = value;

    EXPECT_TRUE(cache.request_inclusive(1, value));
    EXPECT_EQ(value, expected);
}


TEST(MultiCacheInclusive, HitInLowerLevelKeepsPageInHierarchy) {
    // Assign
    multi_cache_t<int, int> cache;

    cache.add_cache<lru_cache_t<int, int>>(1);
    cache.add_cache<lru_cache_t<int, int>>(2);

    int value1;
    int value2;

    // Act
    cache.request_inclusive(1, value1);
    cache.request_inclusive(2, value2);

    // Assert
    int value;

    EXPECT_TRUE(cache.request_inclusive(1, value));
    EXPECT_EQ(value, value1);

    // 1 must still be available after being promoted to L1.
    EXPECT_TRUE(cache.request_inclusive(1, value));
    EXPECT_EQ(value, value1);
}


TEST(MultiCacheInclusive, EvictionInvalidatesUpperLevel) {
    // Assign
    multi_cache_t<int, int> cache;

    cache.add_cache<lru_cache_t<int, int>>(2);
    cache.add_cache<lru_cache_t<int, int>>(2);

    int value1;
    int value2;
    int value3;

    // Act
    cache.request_inclusive(1, value1);
    cache.request_inclusive(2, value2);
    cache.request_inclusive(3, value3);

    // Assert
    int value;

    // Page 1 must be absent from L1 if it was evicted from L2.
    EXPECT_FALSE(cache.request_inclusive(1, value));
    EXPECT_EQ(value, value1);
}
