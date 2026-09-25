#ifndef CACHE_HPP
#define CACHE_HPP

#include <optional>
#include <utility>
#include <cstddef>
#include <list>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <istream>
#include <variant>

int cache_start(int argc, char* argv[]);
size_t run_simulation(size_t capacity, std::istream& is);
int slow_get_page(int key); 

template <typename KeyT, typename ValueT>
class cache_interface {
public:
    virtual ~cache_interface() = default;

    virtual bool lookup(const KeyT& key, ValueT& value) = 0;
    virtual std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) = 0;
    virtual void erase(const KeyT& key) = 0;
};

// ============================================================================
// 1. LRU CACHE (Least Recently Used)
// ============================================================================
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
    void erase(const KeyT& key) {
        auto it = hash_.find(key);
        if (it == hash_.end()) {
            return;
        }
        cache_.erase(it->second);
        hash_.erase(it);
    }
};

// ============================================================================
// 2. LFU CACHE (Least Frequently Used) - O(1)
// ============================================================================
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
                if (min_freq_ == freq) min_freq_++;
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

        // Вставляем новый элемент с частотой 1

        freq_map_[1].push_front({key, value, 1});
        key_map_[key] = freq_map_[1].begin();

        std::optional<std::pair<KeyT, ValueT>> victim;
        // При переполнении вытесняем элемент из группы с минимальной частотой min_freq_
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

        
        min_freq_ = 1;
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
                if (min_freq_ == 0 || pair.first < min_freq_)
                    min_freq_ = pair.first;
            }
        }
    }
};

// ============================================================================
// 3. 2Q CACHE (Two Queues Algorithm)
// ============================================================================
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
    bool lookup(const KeyT& key, ValueT& value) override{
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
    std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) override{

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
        if (hash_.size() == capacity_) {
            if (in_.size() >= kin_ || main_.empty()) {
                hash_.erase(in_.back().key);
                victim = std::make_pair (
                        in_.back().key,
                        in_.back().value
                );
                in_.pop_back();
            } else {
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

       if (it == hash_.end())
           return;

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
};

// ============================================================================
// 4. ARC CACHE (Adaptive Replacement Cache)
// ============================================================================
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
    std::optional<std::pair<KeyT, ValueT>> replace(const KeyT& key) {
        Node old;
        if (!t1_.empty() && (t1_.size() > p_ || (hash_.count(key) && hash_[key].second == 'b' && t1_.size() == p_))) {
            old = t1_.back();
            std::pair<KeyT, ValueT> victim{
                old.key,
                old.value
            };
            t1_.pop_back();
            b1_.push_front(old.key);
            hash_[old.key] = {b1_.begin(), 'a'};
        } else {
            old = t2_.back();
            std::pair<KeyT, ValueT> victim{
                old.key,
                old.value
            };
            t2_.pop_back();
            b2_.push_front(old.key);
            hash_[old.key] = {b2_.begin(), 'b'};
        }
        return std::make_pair(
            old.key,
            old.value
        );
    }

public:
    explicit arc_cache_t(size_t capacity) : c_(capacity), p_(0) {}
    bool lookup(const KeyT& key, ValueT& value) override {
        auto hit = hash_.find(key);

        // 1. HIT в основных списках (T1 или T2)
        if (hit != hash_.end() && (hit->second.second == '1' || hit->second.second == '2')) {

            auto node_it = std::get<TListIt>(hit->second.first);

            value = node_it->value;

            
            if (hit->second.second == '1') t2_.splice(t2_.begin(), t1_, node_it);
            else t2_.splice(t2_.begin(), t2_, node_it);

            // Переводим элемент в список частых T2
            hash_[key] = {t2_.begin(), '2'};
            return true;
        }
        else {
            return false;
        }

    }
    std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) override{
        std::optional<std::pair<KeyT, ValueT>> victim;
        auto hit = hash_.find(key);

        if (hit != hash_.end() &&
            (hit->second.second == '1' || hit->second.second == '2')) {

            auto node_it = std::get<TListIt>(hit->second.first);
            node_it->value = value;

            return std::nullopt;
        }

        
        // 2. HIT в истории B1 (адаптируем p_ в сторону увеличение размера T1)
        if (hit != hash_.end() && hit->second.second == 'a') {
            p_ = std::min(c_, p_ + std::max<size_t>(1, b2_.size() / b1_.size()));
            victim = replace(key);
            auto b1_it = std::get<BListIt>(hit->second.first);
            b1_.erase(b1_it);
            t2_.push_front({key, value});
            hash_[key] = {t2_.begin(), '2'};
            return victim;
        }

        // 3. HIT в истории B2 (адаптируем p_ в сторону увеличения размера T2)
        if (hit != hash_.end() && hit->second.second == 'b') {
            size_t delta = b1_.size() / b2_.size();
            size_t d = delta > 0 ? delta : 1;
            if (p_ > d) {
                p_ = p_ - d;
            }
            else {
                p_ = 0;
            }
            victim = replace(key);
            auto b2_it = std::get<BListIt>(hit->second.first);
            b2_.erase(b2_it);
            t2_.push_front({key, value});
            hash_[key] = {t2_.begin(), '2'};
            return victim;
        }

        // 4. Полный MISS
        if (t1_.size() + b1_.size() == c_) {
            if (t1_.size() < c_) {
                hash_.erase(b1_.back()); 
                b1_.pop_back(); 
                victim = replace(key);
            } else {
                hash_.erase(t1_.back().key); 
                t1_.pop_back();
            }
        } else if (t1_.size() + b1_.size() < c_) {
            size_t total = t1_.size() + t2_.size() + b1_.size() + b2_.size();
            if (total >= c_) {
                if (total == 2 * c_) { 
                    hash_.erase(b2_.back()); 
                    b2_.pop_back(); 
                }
                victim = replace(key);
            }
        }

        t1_.push_front({key, value});
        hash_[key] = {t1_.begin(), '1'};
        return victim;
    }
    void erase(const KeyT& key) override {
        auto it = hash_.find(key);

        if (it == hash_.end())
            return;

        if (it->second.second == '1') {
            auto node_it = std::get<TListIt>(it->second.first);
            t1_.erase(node_it);
            hash_.erase(it);
        }
        else if (it->second.second == '2') {
            auto node_it = std::get<TListIt>(it->second.first);
            t2_.erase(node_it);
            hash_.erase(it);
        }
    }
};

// ============================================================================
// 5. LIRS CACHE (Low Inter-reference Recency Set)
// ============================================================================
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

        // Если физический кеш заполнен,
        // вытесняем самый старый HIR_RES
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
        // ключ известен LIRS, но физически отсутствует в кеше
        if (hit != hash_.end() && hit->second.status == HIR_NON_RES) {

            BlockInfo& info = hit->second;
            info.value = value;
            bool was_in_stack = info.in_stack;
            // Элемент всё ещё находится в стеке S
            if (was_in_stack) {
                S_.splice(S_.begin(), S_, info.stack_it);

                info.stack_it = S_.begin();
                info.status = LIR;

                // Количество LIR не меняется:
                // старый LIR станет HIR_RES
                prune_stack();

                KeyT bottom_lir = S_.back();

                hash_[bottom_lir].status = HIR_RES;

                Q_.push_front(bottom_lir);
                hash_[bottom_lir].q_it = Q_.begin();
            }

            // Элемент уже был удалён из стека S
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

        // ------------------------------------------------
        // НОВЫЙ КЛЮЧ
        // ------------------------------------------------

       

        BlockInfo info;
        info.value = value;

        // Есть место среди LIR
        if (lir_count_ < lir_cap_) {
            info.status = LIR;
            ++lir_count_;
        }

        // LIR уже заполнены → новый элемент HIR_RES
        else {
            info.status = HIR_RES;

            Q_.push_front(key);
            info.q_it = Q_.begin();
        }

        // Добавляем элемент в стек S
        S_.push_front(key);

        info.stack_it = S_.begin();
        info.in_stack = true;

        hash_[key] = info;

        return victim;
    } 
    void erase(const KeyT& key) override {
        auto it = hash_.find(key);

        if (it == hash_.end())
            return;

        BlockInfo& info = it->second;

        // HIR_NON_RES физически уже не находится в кеше.
        if (info.status == HIR_NON_RES)
            return;

        // Удаляем из S_
        if (info.in_stack) {
            S_.erase(info.stack_it);
            info.in_stack = false;
        }

        // HIR_RES дополнительно находится в Q_
        if (info.status == HIR_RES) {
            Q_.erase(info.q_it);
        }

        // Если удаляем LIR, уменьшаем количество LIR
        if (info.status == LIR) {
            --lir_count_;
        }

        hash_.erase(it);
    }
};

#endif // CACHE_HPP
