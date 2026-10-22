#include <gtest/gtest.h>
#include "cache_algo/lirs_cache.hpp"

TEST(LIRSCache, InsertAndLookup) {
    // Assign
    lirs_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    // Assert
    auto val = cache.lookup(1);

    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, 100);
}

TEST(LIRSCache, LookupMissingKey) {
    // Assign
    lirs_cache_t<int, int> cache(4);

    // Assert
    auto val = cache.lookup(1);

    EXPECT_FALSE(val.has_value());
}

TEST(LIRSCache, UpdateExistingKey) {
    // Assign
    lirs_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    // Assert
    auto victim = cache.insert(1, 200);

    EXPECT_FALSE(victim.has_value());

    auto val = cache.lookup(1);

    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, 200);
}

TEST(LIRSCache, FirstElementsBecomeLIR) {
    // Assign
    lirs_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);
    // Assert
    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 4);
    EXPECT_EQ(victim->second, 400);

    auto val1 = cache.lookup(1);
    EXPECT_TRUE(val1.has_value());
    EXPECT_EQ(*val1, 100);

    auto val2 = cache.lookup(2);
    EXPECT_TRUE(val2.has_value());
    EXPECT_EQ(*val2, 200);

    auto val3 = cache.lookup(3);
    EXPECT_TRUE(val3.has_value());
    EXPECT_EQ(*val3, 300);
}

TEST(LIRSCache, HIRHitMovesToLIR) {
    // Assign
    lirs_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);

    // Assert
    auto val4 = cache.lookup(4);
    EXPECT_TRUE(val4.has_value());
    EXPECT_EQ(*val4, 400);

    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);

    EXPECT_TRUE(cache.lookup(4).has_value());
}

TEST(LIRSCache, EraseLIR) {
    // Assign
    lirs_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    cache.erase(1);

    // Assert
    EXPECT_FALSE(cache.lookup(1).has_value());
}

TEST(LIRSCache, EraseHIR) {
    // Assign
    lirs_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);

    cache.erase(4);

    // Assert
    EXPECT_FALSE(cache.lookup(4).has_value());
}

TEST(LIRSCache, EraseMissingKey) {
    // Assign
    lirs_cache_t<int, int> cache(4);

    // Act
    cache.insert(1, 100);

    cache.erase(2);

    // Assert
    auto val = cache.lookup(1);

    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, 100);
}
