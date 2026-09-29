#include <gtest/gtest.h>
#include "cache.hpp"

TEST(LRUCache, InsertAndLookup)
{
    lru_cache_t<int, int> cache(2);

    cache.insert(1, 100);

    int value = 0;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(LRUCache, LookupMissingKey)
{
    lru_cache_t<int, int> cache(2);

    int value = 0;

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(LRUCache, EvictsLeastRecentlyUsed)
{
    lru_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    int value = 0;

    ASSERT_TRUE(cache.lookup(1, value));

    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_TRUE(cache.lookup(3, value));
    EXPECT_FALSE(cache.lookup(2, value));
}

TEST(LRUCache, UpdateExistingKey)
{
    lru_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(1, 200);

    int value = 0;

    ASSERT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 200);
}

TEST(LRUCache, NoEvictionBeforeCapacityIsReached)
{
    lru_cache_t<int, int> cache(2);

    auto victim = cache.insert(1, 100);

    EXPECT_FALSE(victim.has_value());

    victim = cache.insert(2, 200);

    EXPECT_FALSE(victim.has_value());
}

TEST(LRUCache, CapacityOne)
{
    lru_cache_t<int, int> cache(1);

    cache.insert(1, 100);

    auto victim = cache.insert(2, 200);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);

    int value = 0;

    EXPECT_FALSE(cache.lookup(1, value));
    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(LRUCache, LookupUpdatesRecency)
{
    lru_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    int value = 0;

    ASSERT_TRUE(cache.lookup(1, value));

    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);
}

TEST(LRUCache, Erase)
{
    lru_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.erase(1);

    int value = 0;

    EXPECT_FALSE(cache.lookup(1, value));
    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(LRUCache, EraseMissingKey)
{
    lru_cache_t<int, int> cache(2);

    cache.insert(1, 100);

    cache.erase(2);

    int value = 0;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(LRUCache, UpdateExistingKeyWhenFull)
{
    lru_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    auto victim = cache.insert(1, 111);

    EXPECT_FALSE(victim.has_value());

    int value = 0;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 111);

    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}
