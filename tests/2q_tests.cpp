#include <gtest/gtest.h>
#include "cache.hpp"


TEST(TwoQCache, InsertAndLookup)
{
    two_q_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}


TEST(TwoQCache, LookupMissingKey)
{
    two_q_cache_t<int, int> cache(4);

    int value;

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(TwoQCache, LookupMovesFromInToMain)
{
    two_q_cache_t<int, int> cache(4);

    int value;

    cache.insert(1, 100);
    cache.lookup(1, value);

    cache.insert(2, 200);
    cache.lookup(2, value);

    cache.insert(3, 200);
    cache.lookup(3, value);

    cache.insert(4, 200);
    cache.lookup(4, value);

    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);
}

TEST(TwoQCache, LookupUpdatesMainRecency)
{
    two_q_cache_t<int, int> cache(4);

    int value;

    cache.insert(1, 100);
    cache.lookup(1, value);

    cache.insert(2, 200);
    cache.lookup(2, value);

    cache.insert(3, 300);
    cache.lookup(3, value);

    cache.insert(4, 400);
    cache.lookup(4, value);

    // Сейчас MAIN:
    // 4 -> 3 -> 2 -> 1

    cache.lookup(1, value);

    // Теперь MAIN:
    // 1 -> 4 -> 3 -> 2

    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    // Должен уйти самый старый элемент MAIN — 2.
    EXPECT_EQ(victim->first, 2);
    EXPECT_EQ(victim->second, 200);
}

TEST(TwoQCache, UpdateExistingKey)
{
    two_q_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    auto victim = cache.insert(1, 200);

    EXPECT_FALSE(victim.has_value());

    int value;
    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 200);
}

TEST(TwoQCache, UpdateExistingKeyWhenFull)
{
    two_q_cache_t<int, int> cache(2);

    cache.insert(1, 100);
    cache.insert(2, 200);

    auto victim = cache.insert(1, 111);

    EXPECT_FALSE(victim.has_value());

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 111);

    EXPECT_TRUE(cache.lookup(2, value));
    EXPECT_EQ(value, 200);
}

TEST(TwoQCache, EraseFromIn)
{
    two_q_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    cache.erase(1);

    int value;
    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(TwoQCache, EraseFromMain)
{
    two_q_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    int value;
    EXPECT_TRUE(cache.lookup(1, value));

    cache.erase(1);

    EXPECT_FALSE(cache.lookup(1, value));
}

TEST(TwoQCache, EraseMissingKey)
{
    two_q_cache_t<int, int> cache(4);

    cache.insert(1, 100);

    cache.erase(2);

    int value;

    EXPECT_TRUE(cache.lookup(1, value));
    EXPECT_EQ(value, 100);
}

TEST(TwoQCache, EvictsOldestFromIn)
{
    two_q_cache_t<int, int> cache(4);

    cache.insert(1, 100);
    cache.insert(2, 200);
    cache.insert(3, 300);
    cache.insert(4, 400);

    auto victim = cache.insert(5, 500);

    ASSERT_TRUE(victim.has_value());

    EXPECT_EQ(victim->first, 1);
    EXPECT_EQ(victim->second, 100);
}
