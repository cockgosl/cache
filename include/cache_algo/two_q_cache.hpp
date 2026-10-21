#ifndef TWO_Q_CACHE_HPP
#define TWO_Q_CACHE_HPP

#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>

#include "interface/cache_interface.hpp"

template <typename KeyT, typename ValueT>
class two_q_cache_t : public cache_interface<KeyT, ValueT>{
private:
    size_t capacity_; // capacity
    size_t kin_;      // the limit of FIFO

    struct Node {
        KeyT key;
        ValueT value;
    };

    std::list<Node> in_;   // FIFO for new elements (A1in)
    std::list<Node> main_; // LRU for constant elements (Am)

    using ListIt = typename std::list<Node>::iterator;

    // key -> {it, flag (true = in main_, false = in in_)}
    std::unordered_map<KeyT, std::pair<ListIt, bool>> hash_;

public:
    explicit two_q_cache_t(size_t capacity) : capacity_(capacity), kin_(capacity / 4 + 1) {}
    std::optional<ValueT> lookup(const KeyT& key, ValueT& value) override {
        auto hit = hash_.find(key);

        // CACHE HIT
        if (hit != hash_.end()) {
            value = hit->second.first->value;
            // if in LRU main_, refresh it's condition'
            if (hit->second.second) {
                main_.splice(main_.begin(), main_, hit->second.first);
            }
            // if in in_, move in LRU, refresh condition
            else {
                main_.splice(main_.begin(), in_, hit->second.first);
                hit->second.second = true;
            }
            return value;
        }
        else {
            return std::nullopt;
        }
    }
    std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) override {

        auto hit = hash_.find(key);
        if (hit != hash_.end()) {
            hit->second.first->value = value;
            return std::nullopt;
        }

        // new elements in FIFO in_
        in_.push_front({key, value});
        hash_[key] = {in_.begin(), false};

        std::optional<std::pair<KeyT, ValueT>> victim;

        // if cache is filled: eviction from in_, or from main_
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
           // key is in main_
           main_.erase(it->second.first);
       }
       else {
           // key is in in_
           in_.erase(it->second.first);
       }

       hash_.erase(it);
    }
    void print_cache(std::ostream& os = std::cout) const override  {
        os << "IN:\n";

        for (const auto& node : in_) {
            os << "    " << node.key << " -> "
                         << node.value << '\n';
        }

        os << "MAIN:\n";

        for (const auto& node : main_) {
            os << "    " << node.key << " -> "
                         << node.value << '\n';
        }
    }
};

#endif
