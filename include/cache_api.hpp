#ifndef CACHE_API
#define CACHE_API

#include "cache.hpp"
#include <memory>
#include <vector>
#include <cstddef>
#include <iostream>

int slow_get_page(int key) {
    return (key);
}

template <typename KeyT = int>
class multi_cache_t {
private:
    std::vector< std::unique_ptr<cache_interface<KeyT>> > caches_;

    std::vector<size_t> hits_;
    std::vector<size_t> misses_;

public:
    template <typename Cache>
    void add_cache(size_t capacity) {
        caches_.push_back(std::make_unique<Cache>(capacity));

        hits_.push_back(0);
        misses_.push_back(0);
    }

    bool request(KeyT key) {
        // Ищем страницу начиная с L1
        for (size_t i = 0; i < caches_.size(); ++i) {

            if (caches_[i]->lookup(key)) {
                hits_[i]++;

                // Страница найдена на уровне i.
                // Продвигаем её во все более быстрые уровни.
                for (size_t j = 0; j < i; ++j) {
                    caches_[j]->insert(key);
                }

                return true;
            }
            misses_[i]++;
        }


        slow_get_page(key);

        // Добавляем страницу во все уровни
        for (auto& cache : caches_) {
            cache->insert(key);
        }

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
