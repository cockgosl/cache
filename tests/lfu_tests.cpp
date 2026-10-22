#include <gtest/gtest.h>
#include "cache_algo/lfu_cache.hpp"

TEST(LFUCache, InsertAndLookup) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);

    // Assert
    auto val = cache.lookup(1);

    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, 100);
}

TEST(LFUCache, LookupMissingKey) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Assert
    auto val = cache.lookup(1);

    EXPECT_FALSE(val.has_value());
}

TEST(LFUCache, LookupIncreasesFrequency) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    // Assert
    ASSERT_TRUE(cache.lookup(1).has_value());
    ASSERT_TRUE(cache.lookup(1).has_value());

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
    auto val = cache.lookup(1);

    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, 200);
}

TEST(LFUCache, Erase) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.erase(1);

    // Assert
    EXPECT_FALSE(cache.lookup(1).has_value());

    auto val2 = cache.lookup(2);
    EXPECT_TRUE(val2.has_value());
    EXPECT_EQ(*val2, 200);
}

TEST(LFUCache, EraseMissingKey) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);

    cache.erase(2);

    // Assert
    auto val = cache.lookup(1);

    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, 100);
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

    EXPECT_FALSE(cache.lookup(1).has_value());
    auto val2 = cache.lookup(2);
    EXPECT_TRUE(val2.has_value());
    EXPECT_EQ(*val2, 200);
}

TEST(LFUCache, FrequentlyUsedKeyIsNotEvicted) {
    // Assign
    lfu_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.lookup(1);
    cache.lookup(1);

    // Assert
    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);

    auto val1 = cache.lookup(1);
    EXPECT_TRUE(val1.has_value());
    EXPECT_EQ(*val1, 100);

    EXPECT_FALSE(cache.lookup(2).has_value());

    auto val3 = cache.lookup(3);
    EXPECT_TRUE(val3.has_value());
    EXPECT_EQ(*val3, 300);
}
