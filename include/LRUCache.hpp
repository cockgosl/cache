#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <iostream>
#include <cstddef>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>

#include "cache_interface.hpp"

template <typename KeyT = int, typename ValueT = int>
class lru_cache_t : public cache_interface<KeyT, ValueT> {
private:
    size_t capacity_;                          // Максимальная емкость кэша

    struct Node {
        KeyT key;
        ValueT value;
    };
    std::list<Node> cache_;                    // Двусвязный список (порядок использования)
    using ListIt = typename std::list<Node>::iterator; // Псевдоним типа итератора
    std::unordered_map<KeyT , ListIt> hash_;    // Хэш-таблица: Ключ -> Итератор узла в списке

public:
    // Конструктор инициализации емкости
    explicit lru_cache_t(size_t capacity) : capacity_(capacity) {}

    bool lookup(const KeyT& key, ValueT& value) override {
        auto it = hash_.find(key);

        if (it == hash_.end()) {
            return false;
        }

        cache_.splice(cache_.begin(), cache_, it->second);

        value = it->second->value;
        return true;
    }
    std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) override {
        auto it = hash_.find(key);

        if (it != hash_.end()) {
            it->second->value = value;
            return std::nullopt;
        }

        // Добавляем новый элемент в начало
        cache_.push_front({key, value});

        // Запоминаем его итератор в hash_
        hash_[key] = cache_.begin();

        // Если кеш заполнен — удаляем самый старый элемент
        if (cache_.size() > capacity_) {
            auto last = std::prev(cache_.end());
            std::pair<KeyT, ValueT> victim = {
                last->key,
                last->value,
            };
            hash_.erase(cache_.back().key);
            cache_.pop_back();

            return victim;
        }

        return std::nullopt;
    }
    void erase(const KeyT& key) override {
        auto it = hash_.find(key);
        if (it == hash_.end()) {
            return;
        }
        cache_.erase(it->second);
        hash_.erase(it);
    }

    void print_cache() const override {
        for (const auto& node : cache_) {
            std::cout << node.key << " -> " << node.value << '\n';
        }
    }
};

#endif
