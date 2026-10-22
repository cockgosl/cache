#ifndef CACHE_API
#define CACHE_API

#include <memory>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <iostream>
#include <random>

#include "cache_interface.hpp"

template <typename KeyT, typename ValueT>
class slow_memory_t {
private:
    std::unordered_map<KeyT, ValueT> storage_;
    std::mt19937 gen_{std::random_device{}()};
    std::uniform_int_distribution<ValueT> dist_{0, 1000000};

public:
    ValueT get_page(const KeyT& key) {
        auto [it, inserted] = storage_.try_emplace(key, dist_(gen_));
        return it->second;
    }
};

template <typename KeyT, typename ValueT>
class multi_cache_t {
private:
    struct CacheInfo {
        std::unique_ptr<cache_interface<KeyT, ValueT>> cache;
        size_t hits = 0;
        size_t misses = 0;
    };

    std::vector<CacheInfo> caches_;

    slow_memory_t<KeyT, ValueT> slow_memory_;

    void insert_exclusive(size_t level, const KeyT& key, const ValueT& value) {
        if (level >= caches_.size()) {
            return;
        }
        auto victim = caches_[level].cache->insert(key, value);

        if (victim) {
            insert_exclusive(level + 1, victim->first, victim->second);
        }
    }

    void invalidate_upper_levels(size_t level, const KeyT& key) {
        for (size_t i = 0; i < level; i++) {
            caches_[i].cache->erase(key);
        }
    }

    void insert_inclusive(size_t level, const KeyT& key, const ValueT& value) {
        if (level >= caches_.size()) {
            return;
        }

        auto victim = caches_[level].cache->insert(key, value);
        if (victim) {
            invalidate_upper_levels(level, victim->first);
        }

        insert_inclusive(level + 1, key, value);
    }

public:
    template <typename Cache>
    void add_cache(size_t capacity) {
        caches_.emplace_back(std::make_unique<Cache>(capacity), 0, 0);
    }

    void add_cache(std::unique_ptr<cache_interface<KeyT, ValueT>> cache) {
        caches_.emplace_back(std::move(cache), 0, 0);
    }

    bool request_exclusive(const KeyT& key, ValueT& value) {
        for (size_t i = 0; i < caches_.size(); i++) {

            if (auto val = caches_[i].cache->lookup(key)) {
                value = *val;
                caches_[i].hits++;

                if (i == 0) {
                    return true;
                }

                // page was found on level i.
                // delete it from there.
                caches_[i].cache->erase(key);

                // move it to L1 (all evictions go down)
                insert_exclusive(0, key, value);
                return true;
            }
            caches_[i].misses++;
        }

        value = slow_get_page(key);

        // load it into L1.
        // if L1 is full,
        // it will be moved to L2, and to L3 if needed.
        insert_exclusive(0, key, value);

        return false;
    }

    bool request_inclusive(const KeyT& key, ValueT& value) {
        for (size_t i = 0; i < caches_.size(); i++) {
            if (auto val = caches_[i].cache->lookup(key)) {
                value = *val;
                caches_[i].hits++;

                if (i == 0) {
                    return true;
                }
                // at least refresh the data
                insert_inclusive(0, key, value);
                return true;
            }
            caches_[i].misses++;
        }

        value = slow_get_page(key);

        insert_inclusive(0, key, value);

        return false;
    }

    ValueT slow_get_page(const KeyT& key) {
        return slow_memory_.get_page(key);
    }

    size_t hits(size_t level) const {
        return caches_[level].hits;
    }

    size_t misses(size_t level) const {
        return caches_[level].misses;
    }

    size_t size() const {
        return caches_.size();
    }

    // print stats about the first "amount"
    void print_cache(size_t amount) const {
        if (amount > caches_.size()) {
            std::cout << "incorrect amount of caches\n";
            return;
        }

        for (size_t i = 0; i < amount; ++i) {
            std::cout << "L" << i + 1 << ":\n";
            caches_[i].cache->print_cache();
            std::cout << '\n';
        }
    }

    void print_stats() const {
        for (size_t i = 0; i < caches_.size(); ++i) {
            std::cout << "L" << i + 1 << ":\n";
            std::cout << "    Hits: " << caches_[i].hits << '\n';
            std::cout << "    Misses: " << caches_[i].misses << '\n';
        }
    }
};

#endif // CACHE_API
