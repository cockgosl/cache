#ifndef ARC_CACHE_HPP
#define ARC_CACHE_HPP

#include <algorithm>
#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>

#include "interface/cache_interface.hpp"

template <typename KeyT = int, typename ValueT = int>
class arc_cache_t : public cache_interface<KeyT, ValueT> {
private:
    size_t c_; // Емкость
    size_t p_; // Динамический параметр разделения ресурсов

    struct Node {
        KeyT key;
        ValueT value;
    };
    std::list<Node> t1_, t2_; // основные (t1, t2)
    std::list<KeyT> b1_, b2_; // и фантомные списки истории (b1, b2)
    using TListIt = typename std::list<Node>::iterator;
    using BListIt = typename std::list<KeyT>::iterator;
    using ListIt = std::variant<TListIt, BListIt>;
    std::unordered_map<KeyT, std::pair<ListIt, char>> hash_; // '1':t1, '2':t2, 'a':b1, 'b':b2

    // Вспомогательный метод вытеснения
    std::optional<std::pair<KeyT, ValueT>> replace(bool in_b2) {
        if (t1_.empty() && t2_.empty()) {
            return std::nullopt;
        }

        Node old;
        if (!t1_.empty() && (t1_.size() > p_ || (in_b2 && t1_.size() == p_) || t2_.empty())) {
            old = t1_.back();
            t1_.pop_back();
            b1_.push_front(old.key);
            hash_[old.key] = {b1_.begin(), 'a'};
        }
        else if (!t2_.empty()) {
            old = t2_.back();
            t2_.pop_back();
            b2_.push_front(old.key);
            hash_[old.key] = {b2_.begin(), 'b'};
        }
        else {
            return std::nullopt;
        }

        return std::make_pair(old.key, old.value);
    }

public:
    explicit arc_cache_t(size_t capacity) : c_(capacity), p_(0) {}

    bool lookup(const KeyT& key, ValueT& value) override {
        auto hit = hash_.find(key);

        if (hit != hash_.end() && (hit->second.second == '1' || hit->second.second == '2')) {
            auto node_it = std::get<TListIt>(hit->second.first);
            value = node_it->value;

            if (hit->second.second == '1') {
                t2_.splice(t2_.begin(), t1_, node_it);
            }
            else {
                t2_.splice(t2_.begin(), t2_, node_it);
            }

            hash_[key] = {t2_.begin(), '2'};
            return true;
        }
        return false;
    }

    std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) override {
        std::optional<std::pair<KeyT, ValueT>> victim;
        auto hit = hash_.find(key);

        // 1. HIT в T1 или T2
        if (hit != hash_.end() && (hit->second.second == '1' || hit->second.second == '2')) {
            auto node_it = std::get<TListIt>(hit->second.first);
            node_it->value = value;
            if (hit->second.second == '1') {
                t2_.splice(t2_.begin(), t1_, node_it);
            } else {
                t2_.splice(t2_.begin(), t2_, node_it);
            }
            hash_[key] = {t2_.begin(), '2'};
            return std::nullopt;
        }

        // 2. HIT в истории B1
        if (hit != hash_.end() && hit->second.second == 'a') {
            size_t ratio = b1_.empty() ? 1 : (b2_.size() / b1_.size());
            size_t step = ratio > 0 ? ratio : 1;
            p_ = std::min(c_, p_ + step);

            auto b1_it = std::get<BListIt>(hit->second.first);
            b1_.erase(b1_it);
            hash_.erase(hit);

            victim = replace(false);

            t2_.push_front({key, value});
            hash_[key] = {t2_.begin(), '2'};
            return victim;
        }

        // 3. HIT в истории B2
        if (hit != hash_.end() && hit->second.second == 'b') {
            size_t ratio = b2_.empty() ? 1 : (b1_.size() / b2_.size());
            size_t step = ratio > 0 ? ratio : 1;
            if (p_ > step) {
                p_ -= step;
            } else {
                p_ = 0;
            }

            auto b2_it = std::get<BListIt>(hit->second.first);
            b2_.erase(b2_it);
            hash_.erase(hit);

            victim = replace(true);

            t2_.push_front({key, value});
            hash_[key] = {t2_.begin(), '2'};
            return victim;
        }

        // 4. Полный MISS
        if (t1_.size() + b1_.size() >= c_) {
            if (t1_.size() < c_) {
                if (!b1_.empty()) {
                    hash_.erase(b1_.back());
                    b1_.pop_back();
                }
                victim = replace(false);
            }
            else {
                victim = replace(false);    
            }
        }
        else {
            size_t total = t1_.size() + t2_.size() + b1_.size() + b2_.size();
            if (total >= c_) {
                if (total >= 2 * c_) {
                    if (!b2_.empty()) {
                        hash_.erase(b2_.back());
                        b2_.pop_back();
                    } else if (!b1_.empty()) {
                        hash_.erase(b1_.back());
                        b1_.pop_back();
                    }
                }
                victim = replace(false);
            }
        }

        t1_.push_front({key, value});
        hash_[key] = {t1_.begin(), '1'};
        return victim;
    }

    void erase(const KeyT& key) override {
        auto it = hash_.find(key);
        if (it == hash_.end()) return;

        if (it->second.second == '1') {
            auto node_it = std::get<TListIt>(it->second.first);
            t1_.erase(node_it);
        }
        else if (it->second.second == '2') {
            auto node_it = std::get<TListIt>(it->second.first);
            t2_.erase(node_it);
        }
        else if (it->second.second == 'a') {
            auto b_it = std::get<BListIt>(it->second.first);
            b1_.erase(b_it);
        }
        else if (it->second.second == 'b') {
            auto b_it = std::get<BListIt>(it->second.first);
            b2_.erase(b_it);
        }
        hash_.erase(it);
    }

    void print_cache() const override {
        std::cout << "T1:\n";
        for (const auto& node : t1_) {
            std::cout << "    " << node.key << " -> " << node.value << '\n';
        }
        std::cout << "T2:\n";
        for (const auto& node : t2_) {
            std::cout << "    " << node.key << " -> " << node.value << '\n';
        }
        std::cout << "B1:\n";
        for (const auto& key : b1_) {
            std::cout << "    " << key << '\n';
        }
        std::cout << "B2:\n";
        for (const auto& key : b2_) {
            std::cout << "    " << key << '\n';
        }
    }
};

#endif
