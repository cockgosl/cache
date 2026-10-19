#include <gtest/gtest.h>
#include "cache_algo/arc_cache.hpp"

TEST(ARCCache, InsertAndLookup) {
    // Assign
    arc_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    // Assert
    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(ARCCache, LookupMissingKey) {
    // Assign
    arc_cache_t<int, int> cache(4);

    // Assert
    int value;

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(ARCCache, LookupMovesFromT1ToT2) {
    // Assign
    arc_cache_t<int, int> cache(2);

    // Act
    int value;

    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.lookup(1, value);

    // Assert
    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(ARCCache, LookupUpdatesT2Recency) {
    // Assign
    arc_cache_t<int, int> cache(2);

    int value;

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.lookup(1, value);

    cache.insert(3, 300);
    cache.lookup(3, value);

    cache.lookup(1, value);

    // Assert
    auto victim = cache.insert(4, 400);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 3);
    EXPECT_EQ(victim->second, 300);
}

TEST(ARCCache, UpdateExistingKey) {
    // Assign
    arc_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    // Assert
    auto victim = cache.insert(1, 200);

    EXPECT_FALSE(victim.has_value());

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 200);
}

TEST(ARCCache, EvictsFromT1) {
    // Assign
    arc_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);

    // Assert
    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);
}

TEST(ARCCache, B1HitIncreasesP) {
    // Assign
    arc_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    // Assert
    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);

    victim = cache.insert(1, 111);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 111);
}

TEST(ARCCache, B2HitDecreasesP) {
    // Assign
    arc_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.insert(3, 300);

    cache.insert(1, 111);

    // Assert
    auto victim = cache.insert(4, 400);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 111);

    victim = cache.insert(1, 222);

    ASSERT_TRUE(victim.has_value());

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 222);
}

TEST(ARCCache, Erase) {
    // Assign
    arc_cache_t<int, int> cache(4);

    int value;

    // Act
    cache.insert(1, 100);
    cache.erase(1);

    // Assert
    EXPECT_FALSE(cache.lookup(1, value));

    cache.insert(2, 200);
    cache.lookup(2, value);

    cache.erase(2);

    EXPECT_FALSE(cache.lookup(2, value));
}
