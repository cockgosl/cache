#ifndef CACHE_API
#define CACHE_API

#include "cache.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <iostream>
#include <random>



template <typename KeyT = int, typename ValueT = int>
class multi_cache_t {
private:
    std::vector< std::unique_ptr<cache_interface<KeyT, ValueT>> > caches_;

    std::vector<size_t> hits_;
    std::vector<size_t> misses_;

    std::unordered_map<KeyT, ValueT> slow_memory_;

    void insert_to_level(size_t level, const KeyT& key, const ValueT& value) {
        if (level >= caches_.size())
            return;

        auto victim = caches_[level]->insert(key, value);

        if (victim) {
            insert_to_level(level + 1, victim->first, victim->second);
        }
    }
public:
    template <typename Cache>
    void add_cache(size_t capacity) {
        caches_.push_back(std::make_unique<Cache>(capacity));

        hits_.push_back(0);
        misses_.push_back(0);
    }
    

    bool request(const KeyT& key, ValueT& value) {
        // Ищем страницу начиная с L1
        for (size_t i = 0; i < caches_.size(); ++i) {

            if (caches_[i]->lookup(key, value)) {
                hits_[i]++;

                if (i == 0) {
                    return true;
                }

                // Страница была найдена на уровне i.
                // Убираем её оттуда.
                caches_[i]->erase(key);


                // Страница найдена на уровне i.
                // перемещаем в L1(все вытесненные пойдут вниз)
                insert_to_level(0, key, value);
                return true;
            }
            misses_[i]++;
        }


        value = slow_get_page(key);

        // Загружаем её в L1.
        // Если L1 переполнен, вытесненный элемент
        // автоматически пойдёт в L2, затем при необходимости в L3.
        insert_to_level(0, key, value); 

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

    //распечатка информации о первых amount кешах
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
