#ifndef CACHE_HPP
#define CACHE_HPP

#include <cstddef>
#include <list>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <algorithm>

// ============================================================================
// 1. LRU CACHE (Least Recently Used)
// ============================================================================
template <typename KeyT = int>
class lru_cache_t {
private:
    size_t capacity_;                          // Максимальная емкость кэша
    std::list<KeyT> cache_;                    // Двусвязный список (порядок использования)
    using ListIt = typename std::list<KeyT>::iterator; // Псевдоним типа итератора
    std::unordered_map<KeyT, ListIt> hash_;    // Хэш-таблица: Ключ -> Итератор узла в списке

public:
    // Конструктор инициализации емкости
    explicit lru_cache_t(size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) throw std::invalid_argument("Capacity must be > 0");
    }

    // Метод обращения к странице с обновлением приоритета
    template <typename F>
    bool lookup_update(KeyT key, F slow_get_page) {
        // Поиск ключа в хэш-таблице за O(1)
        auto hit = hash_.find(key);

        // CACHE HIT (Попадание)
        if (hit != hash_.end()) {
            // Перемещаем элемент в начало списка (самый свежий)
            cache_.splice(cache_.begin(), cache_, hit->second);
            return true;
        }

        // CACHE MISS (Промах)
        slow_get_page(key); // Загрузка страницы из внешней памяти

        // Если кэш переполнен, вытесняем самый старый элемент из конца списка
        if (cache_.size() == capacity_) {
            hash_.erase(cache_.back());
            cache_.pop_back();
        }

        // Вставляем новый элемент в начало списка и сохраняем итератор в хэш-таблице
        cache_.push_front(key);
        hash_[key] = cache_.begin();
        return false;
    }
};

// ============================================================================
// 2. LFU CACHE (Least Frequently Used) - O(1)
// ============================================================================
template <typename KeyT = int>
class lfu_cache_t {
private:
    size_t capacity_;  // Емкость кэша
    size_t min_freq_;  // Минимальная текущая частота элементов

    // Структура узла хранения ключа и его частоты
    struct Node {
        KeyT key;
        size_t freq;
    };

    using ListIt = typename std::list<Node>::iterator;
    std::unordered_map<KeyT, ListIt> key_map_;             // Ключ -> Итератор узла
    std::unordered_map<size_t, std::list<Node>> freq_map_; // Частота -> Список узлов с этой частотой

public:
    explicit lfu_cache_t(size_t capacity) : capacity_(capacity), min_freq_(0) {
        if (capacity_ == 0) throw std::invalid_argument("Capacity must be > 0");
    }

    template <typename F>
    bool lookup_update(KeyT key, F slow_get_page) {
        auto hit = key_map_.find(key);

        // CACHE HIT
        if (hit != key_map_.end()) {
            auto node_it = hit->second;
            size_t freq = node_it->freq;

            // Удаляем элемент из текущей группы частоты
            freq_map_[freq].erase(node_it);
            if (freq_map_[freq].empty()) {
                freq_map_.erase(freq);
                if (min_freq_ == freq) min_freq_++;
            }

            // Переносим элемент в группу с увеличенной частотой (freq + 1)
            freq_map_[freq + 1].push_front({key, freq + 1});
            key_map_[key] = freq_map_[freq + 1].begin();
            return true;
        }

        // CACHE MISS
        slow_get_page(key);

        // При переполнении вытесняем элемент из группы с минимальной частотой min_freq_
        if (key_map_.size() == capacity_) {
            auto& min_list = freq_map_[min_freq_];
            KeyT evict_key = min_list.back().key;
            min_list.pop_back();
            if (min_list.empty()) freq_map_.erase(min_freq_);
            key_map_.erase(evict_key);
        }

        // Вставляем новый элемент с частотой 1
        min_freq_ = 1;
        freq_map_[1].push_front({key, 1});
        key_map_[key] = freq_map_[1].begin();
        return false;
    }
};

// ============================================================================
// 3. 2Q CACHE (Two Queues Algorithm)
// ============================================================================
template <typename KeyT = int>
class two_q_cache_t {
private:
    size_t capacity_; // Емкость
    size_t kin_;      // Лимит размера FIFO-очереди

    std::list<KeyT> in_;   // FIFO очередь для новых элементов (A1in)
    std::list<KeyT> main_; // LRU список для постоянных элементов (Am)
    using ListIt = typename std::list<KeyT>::iterator;

    // Ключ -> {Итератор, Флаг (true = находится в main_, false = в in_)}
    std::unordered_map<KeyT, std::pair<ListIt, bool>> hash_;

public:
    explicit two_q_cache_t(size_t capacity) : capacity_(capacity), kin_(capacity / 4 + 1) {
        if (capacity_ == 0) throw std::invalid_argument("Capacity must be > 0");
    }

    template <typename F>
    bool lookup_update(KeyT key, F slow_get_page) {
        auto hit = hash_.find(key);

        // CACHE HIT
        if (hit != hash_.end()) {
            // Если элемент находится в LRU-списке main_, обновляем его свежесть
            if (hit->second.second) {
                main_.splice(main_.begin(), main_, hit->second.first);
            }
            return true;
        }

        // CACHE MISS
        slow_get_page(key);

        // Если кэш заполнен: вытесняем из in_, либо из main_
        if (hash_.size() == capacity_) {
            if (in_.size() >= kin_ || main_.empty()) {
                hash_.erase(in_.back());
                in_.pop_back();
            } else {
                hash_.erase(main_.back());
                main_.pop_back();
            }
        }

        // Новые элементы добавляем в FIFO-очередь in_
        in_.push_front(key);
        hash_[key] = {in_.begin(), false};
        return false;
    }
};

// ============================================================================
// 4. ARC CACHE (Adaptive Replacement Cache)
// ============================================================================
template <typename KeyT = int>
class arc_cache_t {
private:
    size_t c_; // Емкость
    size_t p_; // Динамический параметр разделения ресурсов

    std::list<KeyT> t1_, t2_, b1_, b2_; // Основные (t1, t2) и фантомные списки истории (b1, b2)
    using ListIt = typename std::list<KeyT>::iterator;
    std::unordered_map<KeyT, std::pair<ListIt, char>> hash_; // '1':t1, '2':t2, 'a':b1, 'b':b2

    // Вспомогательный метод вытеснения
    void replace(KeyT key) {
        if (!t1_.empty() && (t1_.size() > p_ || (hash_.count(key) && hash_[key].second == 'b' && t1_.size() == p_))) {
            KeyT old = t1_.back();
            t1_.pop_back();
            b1_.push_front(old);
            hash_[old] = {b1_.begin(), 'a'};
        } else {
            KeyT old = t2_.back();
            t2_.pop_back();
            b2_.push_front(old);
            hash_[old] = {b2_.begin(), 'b'};
        }
    }

public:
    explicit arc_cache_t(size_t capacity) : c_(capacity), p_(0) {
        if (c_ == 0) throw std::invalid_argument("Capacity must be > 0");
    }

    template <typename F>
    bool lookup_update(KeyT key, F slow_get_page) {
        auto hit = hash_.find(key);

        // 1. HIT в основных списках (T1 или T2)
        if (hit != hash_.end() && (hit->second.second == '1' || hit->second.second == '2')) {
            if (hit->second.second == '1') t1_.erase(hit->second.first);
            else t2_.erase(hit->second.first);

            t2_.push_front(key); // Переводим элемент в список частых T2
            hash_[key] = {t2_.begin(), '2'};
            return true;
        }

        slow_get_page(key);

        // 2. HIT в истории B1 (адаптируем p_ в сторону увеличение размера T1)
        if (hit != hash_.end() && hit->second.second == 'a') {
            p_ = std::min(c_, p_ + std::max<size_t>(1, b2_.size() / std::max<size_t>(1, b1_.size())));
            replace(key);
            b1_.erase(hit->second.first);
            t2_.push_front(key);
            hash_[key] = {t2_.begin(), '2'};
            return false;
        }

        // 3. HIT в истории B2 (адаптируем p_ в сторону увеличения размера T2)
        if (hit != hash_.end() && hit->second.second == 'b') {
            size_t delta = b1_.size() / std::max<size_t>(1, b2_.size());
            p_ = (p_ > (delta > 0 ? delta : 1)) ? p_ - (delta > 0 ? delta : 1) : 0;
            replace(key);
            b2_.erase(hit->second.first);
            t2_.push_front(key);
            hash_[key] = {t2_.begin(), '2'};
            return false;
        }

        // 4. Полный MISS
        if (t1_.size() + b1_.size() == c_) {
            if (t1_.size() < c_) {
                hash_.erase(b1_.back()); b1_.pop_back(); replace(key);
            } else {
                hash_.erase(t1_.back()); t1_.pop_back();
            }
        } else if (t1_.size() + b1_.size() < c_) {
            size_t total = t1_.size() + t2_.size() + b1_.size() + b2_.size();
            if (total >= c_) {
                if (total == 2 * c_) { hash_.erase(b2_.back()); b2_.pop_back(); }
                replace(key);
            }
        }

        t1_.push_front(key);
        hash_[key] = {t1_.begin(), '1'};
        return false;
    }
};

// ============================================================================
// 5. LIRS CACHE (Low Inter-reference Recency Set)
// ============================================================================
template <typename KeyT = int>
class lirs_cache_t {
private:
    size_t capacity_;
    size_t lir_cap_;

    enum Status { LIR, HIR_RES, HIR_NON_RES };

    struct BlockInfo {
        Status status;
        bool in_stack;
        typename std::list<KeyT>::iterator stack_it;
        typename std::list<KeyT>::iterator q_it;
    };

    std::list<KeyT> S_; // Стек S
    std::list<KeyT> Q_; // Очередь Q
    std::unordered_map<KeyT, BlockInfo> hash_;
    size_t lir_count_ = 0;

    // Очистка низа стека S от не-LIR элементов
    void prune_stack() {
        while (!S_.empty()) {
            KeyT k = S_.back();
            auto it = hash_.find(k);
            if (it == hash_.end() || it->second.status != LIR) {
                if (it != hash_.end()) it->second.in_stack = false;
                S_.pop_back();
            } else {
                break;
            }
        }
    }

public:
    explicit lirs_cache_t(size_t capacity) : capacity_(capacity), lir_cap_(capacity > 1 ? capacity - 1 : 1) {
        if (capacity_ == 0) throw std::invalid_argument("Capacity must be > 0");
    }

    template <typename F>
    bool lookup_update(KeyT key, F slow_get_page) {
        auto hit = hash_.find(key);

        // CACHE HIT
        if (hit != hash_.end() && hit->second.status != HIR_NON_RES) {
            BlockInfo& info = hit->second;
            if (info.status == LIR) {
                S_.erase(info.stack_it);
                S_.push_front(key);
                info.stack_it = S_.begin();
                prune_stack();
            } else if (info.status == HIR_RES) {
                S_.push_front(key);
                bool was_in_stack = info.in_stack;
                if (was_in_stack) S_.erase(info.stack_it);
                info.stack_it = S_.begin();
                info.in_stack = true;

                if (was_in_stack) { // Переводим HIR -> LIR
                    info.status = LIR;
                    Q_.erase(info.q_it);

                    KeyT bottom_lir = S_.back();
                    hash_[bottom_lir].status = HIR_RES;
                    Q_.push_front(bottom_lir);
                    hash_[bottom_lir].q_it = Q_.begin();
                    prune_stack();
                } else {
                    Q_.erase(info.q_it);
                    Q_.push_front(key);
                    info.q_it = Q_.begin();
                }
            }
            return true;
        }

        // CACHE MISS
        slow_get_page(key);

        // Вытеснение при переполнении
        if (lir_count_ >= lir_cap_ && hash_.size() >= capacity_) {
            if (!Q_.empty()) {
                KeyT victim = Q_.back();
                Q_.pop_back();
                hash_[victim].status = HIR_NON_RES;
            }
        }

        // Добавление нового элемента
        BlockInfo info;
        if (lir_count_ < lir_cap_) {
            info.status = LIR;
            lir_count_++;
        } else {
            info.status = HIR_RES;
            Q_.push_front(key);
            info.q_it = Q_.begin();
        }
        S_.push_front(key);
        info.stack_it = S_.begin();
        info.in_stack = true;
        hash_[key] = info;

        return false;
    }
};

#endif // CACHE_HPP
