#include <gtest/gtest.h>
#include "cache.hpp"

TEST(LFUCache, InsertAndLookup)
{
    lfu_cache_t<int, int> cache(2);

    cache.insert(1, 100);

    int value = 0;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(LFUCache, LookupMissingKey)
{
    lfu_cache_t<int, int> cache(2);

    int value = 0;

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(LFUCache, LookupIncreasesFrequency)
{
    lfu_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    int value = 0;

    ASSERT_TRUE(cache.lookup(1, value));
    ASSERT_TRUE(cache.lookup(1, value));

    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);
}

TEST(LFUCache, UpdateExistingKey)
{
    lfu_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(1, 200);

    int value = 0;

    ASSERT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 200);
}

TEST(LFUCache, UpdateDoesNotEvict)
{
    lfu_cache_t<int, int> cache(2);

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

TEST(LFUCache, Erase)
{
    lfu_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    cache.erase(1);

    int value = 0;

    EXPECT_FALSE(cache.lookup(1, value));

    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(LFUCache, EraseMissingKey)
{
    lfu_cache_t<int, int> cache(2);

    cache.insert(1, 100);

    cache.erase(2);

    int value = 0;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(LFUCache, EvictsOldestAmongEqualFrequencies)
{
    lfu_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    auto victim = cache.insert(3, 300);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);
}

TEST(LFUCache, CapacityOne)
{
    lfu_cache_t<int, int> cache(1);

    cache.insert(1, 100);

    auto victim = cache.insert(2, 200);

    ASSERT_TRUE(victim.has_value());
    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);

    int value;
    EXPECT_FALSE(cache.lookup(1, value));
    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(LFUCache, UpdateExistingKeyWhenFull)
{
    lfu_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    auto victim = cache.insert(1, 999);

    EXPECT_FALSE(victim.has_value());

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 999);

    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(LFUCache, FrequentlyUsedKeyIsNotEvicted)
{
    lfu_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    int value;

    // Увеличиваем частоту ключа 1:
    // 1 -> freq 3
    cache.lookup(1, value);
    cache.lookup(1, value);

    // 2 остаётся с freq 1
    // Добавляем новый ключ — должен вытесниться 2
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
