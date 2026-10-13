#ifndef LFU_CACHE_HPP
#define LFU_CACHE_HPP

#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>

#include "cache_interface.hpp"

template <typename KeyT = int, typename ValueT = int>
class lfu_cache_t : public cache_interface<KeyT, ValueT> {
private:
    size_t capacity_;  // Емкость кэша
    size_t min_freq_;  // Минимальная текущая частота элементов

    // Структура узла хранения ключа и его частоты
    struct Node {
        KeyT key;
        ValueT value;
        size_t freq;
    };

    using ListIt = typename std::list<Node>::iterator;
    std::unordered_map<KeyT, ListIt> key_map_;             // Ключ -> Итератор узла
    std::unordered_map<size_t, std::list<Node>> freq_map_; // Частота -> Список узлов с этой частотой

public:
    explicit lfu_cache_t(size_t capacity) : capacity_(capacity), min_freq_(0) {}

    bool lookup(const KeyT& key, ValueT& value) override {
        auto hit = key_map_.find(key);

        // CACHE HIT
        if (hit != key_map_.end()) {
            auto node_it = hit->second;
            value = node_it->value;
            size_t freq = node_it->freq;

            // Удаляем элемент из текущей группы частоты
            freq_map_[freq].erase(node_it);
            if (freq_map_[freq].empty()) {
                freq_map_.erase(freq);
                if (min_freq_ == freq) {
                    min_freq_++;
                }
            }

            // Переносим элемент в группу с увеличенной частотой (freq + 1)
            freq_map_[freq + 1].push_front({key, value, freq + 1});
            key_map_[key] = freq_map_[freq + 1].begin();

            return true;
        }
        else {
            return false;
        }
    }
    std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) override {
        auto it = key_map_.find(key);
        if (it != key_map_.end()) {
            it->second->value = value;
            return std::nullopt;
        }

        freq_map_[1].push_front({key, value, 1});
        key_map_[key] = freq_map_[1].begin();

        // Устанавливаем min_freq_ = 1 ДО проверки на вытеснение
        min_freq_ = 1;

        std::optional<std::pair<KeyT, ValueT>> victim;
        if (key_map_.size() > capacity_) {
            auto& min_list = freq_map_[min_freq_];
            victim = std::make_pair (
                min_list.back().key,
                min_list.back().value
            );
            min_list.pop_back();
            if (min_list.empty()) {
                freq_map_.erase(min_freq_);
            }
            key_map_.erase(victim->first);
        }

        return victim;
    }
    void erase(const KeyT& key) override {
        auto it = key_map_.find(key);
        if (it == key_map_.end()) {
            return;
        }
        auto node_it = it->second;
        size_t freq = node_it->freq;

        freq_map_[freq].erase(node_it);
        if (freq_map_[freq].empty()) {
            freq_map_.erase(freq);
        }
        key_map_.erase(it);
        if (key_map_.empty()) {
            min_freq_ = 0;
        }
        else if (freq == min_freq_ && freq_map_.find(freq) == freq_map_.end()) {
            min_freq_ = 0;
            for (const auto& pair : freq_map_) {
                if (min_freq_ == 0 || pair.first < min_freq_) {
                    min_freq_ = pair.first;
                }
            }
        }
    }
    void print_cache() const override {

        for (const auto& [freq, list] : freq_map_) {
            std::cout << "freq " << freq << ":\n";

            for (const auto& node : list) {
                std::cout << "    "
                        << node.key << " -> "
                        << node.value << '\n';
            }
        }
    }
};

#endif
