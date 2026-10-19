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
class multi_cache_t {
private:
    std::vector< std::unique_ptr<cache_interface<KeyT, ValueT>> > caches_;

    std::vector<size_t> hits_;
    std::vector<size_t> misses_;

    std::unordered_map<KeyT, ValueT> slow_memory_;

    void insert_exclusive(size_t level, const KeyT& key, const ValueT& value) {
        if (level >= caches_.size()) {
            return;
        }
        auto victim = caches_[level]->insert(key, value);

        if (victim) {
            insert_exclusive(level + 1, victim->first, victim->second);
        }
    }

    void invalidate_upper_levels(size_t level, const KeyT& key) {
        for (size_t i = 0; i < level; i++) {
            caches_[i]->erase(key);
        }
    }

    void insert_inclusive(size_t level, const KeyT& key, const ValueT& value) {
        if (level >= caches_.size()) {
            return;
        } 

        auto victim = caches_[level]->insert(key, value);
        if (victim) {
            invalidate_upper_levels(level, victim->first); 
        }

        insert_inclusive(level+1, key, value);
    }
public:
    template <typename Cache>
    void add_cache(size_t capacity) {
        caches_.push_back(std::make_unique<Cache>(capacity));

        hits_.push_back(0);
        misses_.push_back(0);
    }

    void add_cache(std::unique_ptr<cache_interface<KeyT, ValueT>> cache) {
        caches_.push_back(std::move(cache));
        hits_.push_back(0);
        misses_.push_back(0);
    }

    bool request_exclusive(const KeyT& key, ValueT& value) {
        for (size_t i = 0; i < caches_.size(); i++) {

            if (caches_[i]->lookup(key, value)) {
                hits_[i]++;

                if (i == 0) {
                    return true;
                }

                // page was found on the level i.
                // delere it from there.
                caches_[i]->erase(key);

                // move it in L1(all evictions are going down)
                insert_exclusive(0, key, value);
                return true;
            }
            misses_[i]++;
        }

        value = slow_get_page(key);

        // load it in L1.
        // if L1 is filled
        // it will be moved in L2, if needed in L3.
        insert_exclusive(0, key, value);

        return false;
    }

    bool request_inclusive(const KeyT& key, ValueT& value) {
        for (size_t i = 0; i < caches_.size(); i++) {
            if (caches_[i]->lookup(key, value)) {
                hits_[i]++;
                if (i == 0) {
                    return true;
                }
                // at least renew the data
                insert_inclusive(0, key, value);
                return true;
            }
            misses_[i]++;
        }
        
        value = slow_get_page(key);

        insert_inclusive(0, key, value);

        return false;
    }

    ValueT slow_get_page(const KeyT& key) {
        auto it = slow_memory_.find(key);

        if (it != slow_memory_.end()) {
            return it->second;
        }

        static std::mt19937 gen(std::random_device{}());
        static std::uniform_int_distribution<ValueT> dist(0, 1000000);

        ValueT value = dist(gen);
        slow_memory_[key] = value;

        return value;
    }
    
    size_t hits(size_t level) const {
        return hits_[level];
    }   

    size_t misses(size_t level) const {
        return misses_[level];
    }

    size_t size() {
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
            caches_[i]->print_cache();
            std::cout << '\n';
        }
    }

    void print_stats() const {
        for (size_t i = 0; i < caches_.size(); ++i) {
            std::cout << "L" << i + 1 << ":\n";
            std::cout << "    Hits: " << hits_[i] << '\n';
            std::cout << "    Misses: " << misses_[i] << '\n';
        }
    }
};

#endif //CACHE_API
