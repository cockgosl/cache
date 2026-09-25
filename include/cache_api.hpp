#ifndef CACHE_API
#define CACHE_API

#include "cache.hpp"
#include <memory>
#include <vector>
#include <cstddef>
#include <iostream>



template <typename KeyT = int, typename ValueT = int>
class multi_cache_t {
private:
    std::vector< std::unique_ptr<cache_interface<KeyT, ValueT>> > caches_;

    std::vector<size_t> hits_;
    std::vector<size_t> misses_;

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
    void print_stats() const {
        for (size_t i = 0; i < caches_.size(); ++i) {
            std::cout << "L" << i + 1 << ":\n";
            std::cout << "    Hits: " << hits_[i] << '\n';
            std::cout << "    Misses: " << misses_[i] << '\n';
        }
    }
};

#endif //CACHE_API
