#ifndef TWO_Q_CACHE_HPP
#define TWO_Q_CACHE_HPP

#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>

#include "cache_interface.hpp"

template <typename KeyT = int, typename ValueT = int>
class two_q_cache_t : public cache_interface<KeyT, ValueT>{
private:
    size_t capacity_; // Емкость
    size_t kin_;      // Лимит размера FIFO-очереди

    struct Node {
        KeyT key;
        ValueT value;
    };

    std::list<Node> in_;   // FIFO очередь для новых элементов (A1in)
    std::list<Node> main_; // LRU список для постоянных элементов (Am)

    using ListIt = typename std::list<Node>::iterator;

    // Ключ -> {Итератор, Флаг (true = находится в main_, false = в in_)}
    std::unordered_map<KeyT, std::pair<ListIt, bool>> hash_;

public:
    explicit two_q_cache_t(size_t capacity) : capacity_(capacity), kin_(capacity / 4 + 1) {}
    bool lookup(const KeyT& key, ValueT& value) override {
        auto hit = hash_.find(key);

        // CACHE HIT
        if (hit != hash_.end()) {
            value = hit->second.first->value;
            // Если элемент находится в LRU-списке main_, обновляем его свежесть
            if (hit->second.second) {
                main_.splice(main_.begin(), main_, hit->second.first);
            }
            // Если элемент в in, просто перемещаем его в LRU и обновляем статус
            else {
                main_.splice(main_.begin(), in_, hit->second.first);
                hit->second.second = true;
            }
            return true;
        }
        else {
            return false;
        }
    }
    std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) override {

        auto hit = hash_.find(key);
        if (hit != hash_.end()) {
            hit->second.first->value = value;
            return std::nullopt;
        }

        // Новые элементы добавляем в FIFO-очередь in_
        in_.push_front({key, value});
        hash_[key] = {in_.begin(), false};

        std::optional<std::pair<KeyT, ValueT>> victim;

        // Если кэш заполнен: вытесняем из in_, либо из main_
        if (hash_.size() > capacity_) {
            if (in_.size() >= kin_ || main_.empty()) {
                hash_.erase(in_.back().key);
                victim = std::make_pair (
                        in_.back().key,
                        in_.back().value
                );
                in_.pop_back();
            }
            else {
                hash_.erase(main_.back().key);
                victim = std::make_pair (
                        main_.back().key,
                        main_.back().value
                );
                main_.pop_back();
            }
        }

        return victim;
    }
    void erase(const KeyT& key) override {
       auto it = hash_.find(key);

       if (it == hash_.end()) {

           return;

       }
       if (it->second.second) {
           // Ключ находится в main_
           main_.erase(it->second.first);
       }
       else {
           // Ключ находится в in_
           in_.erase(it->second.first);
       }

       hash_.erase(it);
    }
    void print_cache() const override {
        std::cout << "IN:\n";

        for (const auto& node : in_) {
            std::cout << "    "
                      << node.key << " -> "
                      << node.value << '\n';
        }

        std::cout << "MAIN:\n";

        for (const auto& node : main_) {
            std::cout << "    "
                      << node.key << " -> "
                      << node.value << '\n';
        }
    }
};

#endif
