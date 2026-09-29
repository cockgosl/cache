#include <gtest/gtest.h>
#include "cache.hpp"

TEST(LIRSCache, InsertAndLookup)
{
    lirs_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(LIRSCache, LookupMissingKey)
{
    lirs_cache_t<int, int> cache(4);

    int value;

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(LIRSCache, UpdateExistingKey)
{
    lirs_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    auto victim = cache.insert(1, 200);

    EXPECT_FALSE(victim.has_value());

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 200);
}

TEST(LIRSCache, FirstElementsBecomeLIR)
{
    lirs_cache_t<int, int> cache(4);

    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);

    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 4);
    EXPECT_EQ(victim->second, 400);

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);

    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);

    EXPECT_TRUE(cache.lookup(3, value));
    EXPECT_EQ(value, 300);
}

TEST(LIRSCache, HIRHitMovesToLIR)
{
    lirs_cache_t<int, int> cache(4);

    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);

    int value;

    EXPECT_TRUE(cache.lookup(4, value));
    EXPECT_EQ(value, 400);

    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);

    EXPECT_TRUE(cache.lookup(4, value));
    EXPECT_EQ(value, 400);
}

TEST(LIRSCache, EraseLIR)
{
    lirs_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    cache.erase(1);

    int value;

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(LIRSCache, EraseHIR)
{
    lirs_cache_t<int, int> cache(4);

    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);

    cache.erase(4);

    int value;

    EXPECT_FALSE(cache.lookup(4, value));
}

TEST(LIRSCache, EraseMissingKey)
{
    lirs_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    cache.erase(2);

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

