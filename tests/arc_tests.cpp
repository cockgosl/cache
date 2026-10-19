#include <gtest/gtest.h>
#include "cache_algo/arc_cache.hpp"

TEST(ARCCache, InsertAndLookup)
{
    arc_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(ARCCache, LookupMissingKey)
{
    arc_cache_t<int, int> cache(4);

    int value;

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(ARCCache, LookupMovesFromT1ToT2)
{
    arc_cache_t<int, int> cache(2);

    int value;

    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.lookup(1, value);

    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(ARCCache, LookupUpdatesT2Recency)
{
    arc_cache_t<int, int> cache(2);

    int value;

    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.lookup(1, value);

    cache.insert(3, 300);
    cache.lookup(3, value);

    // T2: [3, 1]
    cache.lookup(1, value);

    // T2: [1, 3]
    auto victim = cache.insert(4, 400);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 3);
    EXPECT_EQ(victim->second, 300);
}

TEST(ARCCache, UpdateExistingKey)
{
    arc_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    auto victim = cache.insert(1, 200);

    EXPECT_FALSE(victim.has_value());

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 200);
}

TEST(ARCCache, EvictsFromT1)
{
    arc_cache_t<int, int> cache(4);

    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);

    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);
}

TEST(ARCCache, B1HitIncreasesP)
{
    arc_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    // 1 is evicted from T1 in B1
    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);

    // 1 is in B1
    // p is to be increased and 1 is to be moved in T2
    victim = cache.insert(1, 111);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 111);
}

TEST(ARCCache, B2HitDecreasesP)
{
    arc_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    // 1 -> B1
    cache.insert(3, 300);

    // B1 hit: p is increased
    cache.insert(1, 111);

    // 2 is evicted: T2 -> B2
    auto victim = cache.insert(4, 400);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 111);

    // 1 is in B2.
    // B2 hit reduces p.
    victim = cache.insert(1, 222);

    ASSERT_TRUE(victim.has_value());

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 222);
}

TEST(ARCCache, Erase)
{
    arc_cache_t<int, int> cache(4);

    int value;

    // T1
    cache.insert(1, 100);
    cache.erase(1);

    EXPECT_FALSE(cache.lookup(1, value));

    // move it in T2
    cache.insert(2, 200);
    cache.lookup(2, value);

    cache.erase(2);

    EXPECT_FALSE(cache.lookup(2, value));
}
