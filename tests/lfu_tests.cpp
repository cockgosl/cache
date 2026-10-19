#include <gtest/gtest.h>
#include "cache_algo/lfu_cache.hpp"
TEST(LFUCache, InsertAndLookup) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);

    // Assert
    int value = 0;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(LFUCache, LookupMissingKey) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Assert
    int value = 0;

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(LFUCache, LookupIncreasesFrequency) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    // Assert
    int value = 0;

    ASSERT_TRUE(cache.lookup(1, value));
    ASSERT_TRUE(cache.lookup(1, value));

    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);
}

TEST(LFUCache, UpdateExistingKey) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(1, 200);

    // Assert
    int value = 0;

    ASSERT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 200);
}

TEST(LFUCache, Erase) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.erase(1);

    // Assert
    int value = 0;

    EXPECT_FALSE(cache.lookup(1, value));

    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(LFUCache, EraseMissingKey) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);

    cache.erase(2);

    // Assert
    int value = 0;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(LFUCache, EvictsOldestAmongEqualFrequencies) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    // Assert
    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);
}

TEST(LFUCache, CapacityOne) {
    // Assign
    lfu_cache_t<int, int> cache(1);

    // Act
    cache.insert(1, 100);

    // Assert
    auto victim = cache.insert(2, 200);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);

    int value;
    EXPECT_FALSE(cache.lookup(1, value));
    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(LFUCache, FrequentlyUsedKeyIsNotEvicted) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    int value;

    cache.lookup(1, value);
    cache.lookup(1, value);

    // Assert
    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);

    EXPECT_FALSE(cache.lookup(2, value));

    EXPECT_TRUE(cache.lookup(3, value));
    EXPECT_EQ(value, 300);
}
