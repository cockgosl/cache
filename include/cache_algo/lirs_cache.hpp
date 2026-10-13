#ifndef LIRS_CACHE_HPP
#define LIRS_CACHE_HPP

#include "interface/cache_interface.hpp"

#include <iostream>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>

template <typename KeyT = int, typename ValueT = int>
class lirs_cache_t : public cache_interface<KeyT, ValueT> {
private:
    size_t capacity_;
    size_t lir_cap_;

    enum Status { LIR, HIR_RES, HIR_NON_RES };

    struct BlockInfo {
        Status status;
        bool in_stack;
        typename std::list<KeyT>::iterator stack_it;
        typename std::list<KeyT>::iterator q_it;
        ValueT value;
    };

    std::list<KeyT> S_; // stack S
    std::list<KeyT> Q_; // que Q
    std::unordered_map<KeyT, BlockInfo> hash_;
    size_t lir_count_ = 0;

    // clear the bottom of S from non LIR elements
    void prune_stack() {
        while (!S_.empty()) {
            KeyT k = S_.back();
            auto it = hash_.find(k);
            if (it == hash_.end() || it->second.status != LIR) {
                if (it != hash_.end()) {
                    it->second.in_stack = false;
                }
                S_.pop_back();
            }
            else {
                break;
            }
        }
    }

public:
    explicit lirs_cache_t(size_t capacity) : capacity_(capacity), lir_cap_(capacity > 1 ? capacity - 1 : 1) {}
    bool lookup(const KeyT& key, ValueT& value) override {
        auto hit = hash_.find(key);

        // CACHE HIT
        if (hit != hash_.end() && hit->second.status != HIR_NON_RES) {
            BlockInfo& info = hit->second;
            value = info.value;
            if (info.status == LIR) {
                S_.splice(S_.begin(), S_, info.stack_it);
                prune_stack();
            }
            else if (info.status == HIR_RES) {
                bool was_in_stack = info.in_stack;
                if (was_in_stack) {
                    S_.splice(S_.begin(), S_, info.stack_it);
                }
                else {
                    S_.push_front(key);
                }
                info.stack_it = S_.begin();
                info.in_stack = true;

                if (was_in_stack) { // HIR -> LIR
                    info.status = LIR;
                    Q_.erase(info.q_it);

                    KeyT bottom_lir = S_.back();
                    hash_[bottom_lir].status = HIR_RES;
                    Q_.push_front(bottom_lir);
                    hash_[bottom_lir].q_it = Q_.begin();
                    prune_stack();
                }
                else {
                    Q_.erase(info.q_it);
                    Q_.push_front(key);
                    info.q_it = Q_.begin();
                }
            }
            return true;
        }
        else {
            return false;
        }

    }

    std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) override {
        auto hit = hash_.find(key);

        if (hit != hash_.end() && (hit->second.status != HIR_NON_RES)) {
            hit->second.value = value;
            return std::nullopt;
        }

        std::optional<std::pair<KeyT, ValueT>> victim;

        // if cache is filled,
        // evict the last HIR_RES
        if (lir_count_ + Q_.size() >= capacity_) {
            if (!Q_.empty()) {
                KeyT victim_key = Q_.back();
                Q_.pop_back();

                victim = std::make_pair (
                    victim_key,
                    hash_[victim_key].value
                );

                hash_[victim_key].status = HIR_NON_RES;
            }
        }

        // HIR_NON_RES:
        // the key is LIRS, but it's not in cache
        if (hit != hash_.end() && hit->second.status == HIR_NON_RES) {

            BlockInfo& info = hit->second;
            info.value = value;
            bool was_in_stack = info.in_stack;
            // it's in S
            if (was_in_stack) {
                S_.splice(S_.begin(), S_, info.stack_it);

                info.stack_it = S_.begin();
                info.status = LIR;

                // the amount of LIR is not changed:
                // the old LIR becomes HIR_RES
                prune_stack();

                KeyT bottom_lir = S_.back();

                hash_[bottom_lir].status = HIR_RES;

                Q_.push_front(bottom_lir);
                hash_[bottom_lir].q_it = Q_.begin();
            }

            // the element was deleted from S
            else {
                S_.push_front(key);

                info.stack_it = S_.begin();
                info.in_stack = true;

                info.status = HIR_RES;

                Q_.push_front(key);
                info.q_it = Q_.begin();
            }

            return victim;
        }

        //New key
        BlockInfo info;
        info.value = value;

        // there is space for a new LIR
        if (lir_count_ < lir_cap_) {
            info.status = LIR;
            ++lir_count_;
        }

        // there is no space → HIR_RES
        else {
            info.status = HIR_RES;

            Q_.push_front(key);
            info.q_it = Q_.begin();
        }

        // add the element in S
        S_.push_front(key);

        info.stack_it = S_.begin();
        info.in_stack = true;

        hash_[key] = info;

        return victim;
    }
    void erase(const KeyT& key) override {
        auto it = hash_.find(key);

        if (it == hash_.end()) {

            return;

        }
        BlockInfo& info = it->second;

        // HIR_NON_RES is already not in cache.
        if (info.status == HIR_NON_RES) {
            return;
        }
        // evict from S_
        if (info.in_stack) {
            S_.erase(info.stack_it);
            info.in_stack = false;
        }

        // HIR_RES is in Q_
        if (info.status == HIR_RES) {
            Q_.erase(info.q_it);
        }

        // reduce the amount of LIR
        if (info.status == LIR) {
            --lir_count_;
        }

        hash_.erase(it);
    }
    void print_cache() const override {
        std::cout << "S:\n";

        for (const auto& key : S_) {
            auto it = hash_.find(key);

            if (it != hash_.end()) {
                std::cout << "    "
                          << key << " -> "
                          << it->second.value << '\n';
            }
        }

        std::cout << "Q:\n";

        for (const auto& key : Q_) {
            auto it = hash_.find(key);

            if (it != hash_.end()) {
                std::cout << "    "
                          << key << " -> "
                          << it->second.value << '\n';
            }
        }
    }
};

#endif
