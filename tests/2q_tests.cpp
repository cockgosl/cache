#include <gtest/gtest.h>
#include "cache_algo/two_q_cache.hpp"

TEST(TwoQCache, LookupMissingKey) {
    // Assign
    two_q_cache_t<int, int> cache(4);

    int value;

    // Assert
    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(TwoQCache, LookupUpdatesMainRecency) {
    // Assign
    two_q_cache_t<int, int> cache(4);

    int value;
    
    // Act
    cache.insert(1, 100);
    cache.lookup(1, value);

    cache.insert(2, 200);
    cache.lookup(2, value);

    cache.insert(3, 300);
    cache.lookup(3, value);

    cache.insert(4, 400);
    cache.lookup(4, value);

    cache.lookup(1, value);

    // Assert
    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);
}

TEST(TwoQCache, UpdateExistingKey) {
    // Assign
    two_q_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    // Assert
    auto victim = cache.insert(1, 200);

    EXPECT_FALSE(victim.has_value());

    int value;
    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 200);
}

TEST(TwoQCache, UpdateExistingKeyWhenFull) {
    // Assign
    two_q_cache_t<int, int> cache(2);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);

    // Assert
    auto victim = cache.insert(1, 111);

    EXPECT_FALSE(victim.has_value());

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 111);

    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(TwoQCache, EraseFromIn) {
    // Assign
    two_q_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    cache.erase(1);

    // Assert
    int value;
    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(TwoQCache, EraseFromMain) {
    // Assign
    two_q_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    // Assert
    int value;
    EXPECT_TRUE(cache.lookup(1, value));

    cache.erase(1);

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(TwoQCache, EraseMissingKey) {
    // Assign
    two_q_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    cache.erase(2);

    // Assert
    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(TwoQCache, EvictsOldestFromIn) {
    // Assign
    two_q_cache_t<int, int> cache(4);

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
