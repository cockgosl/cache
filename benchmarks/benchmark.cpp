#include "cache.hpp"
#include "cache_api.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>
#include <functional>
#include <memory>


// ИДЕАЛЬНЫЙ АЛГОРИТМ БЕЛАДИ (Belady's OPT)
size_t belady_opt_hits(size_t capacity, const std::vector<int>& requests) {
    if (capacity == 0 || requests.empty()) return 0;

    size_t n = requests.size();
    std::vector<size_t> next_pos(n, n + 1);
    std::unordered_map<int, size_t> last_seen;

    for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
        auto it = last_seen.find(requests[i]);
        if (it != last_seen.end()) {
            next_pos[i] = it->second;
        }
        last_seen[requests[i]] = i;
    }

    std::unordered_map<int, size_t> cache;
    size_t hits = 0;

    for (size_t i = 0; i < n; ++i) {
        int key = requests[i];
        size_t next_occ = next_pos[i];

        auto it = cache.find(key);
        if (it != cache.end()) {
            hits++;
            it->second = next_occ;
        } else {
            if (cache.size() >= capacity) {
                int victim_key = -1;
                size_t furthest = 0;
                for (const auto& [c_key, c_next] : cache) {
                    if (c_next > furthest) {
                        furthest = c_next;
                        victim_key = c_key;
                    }
                }
                if (victim_key != -1) cache.erase(victim_key);
            }
            cache[key] = next_occ;
        }
    }
    return hits;
}

// ГЕНЕРАТОРЫ НАГРУЗОК
namespace Workloads {
    std::vector<int> generate_loop(size_t num_requests, size_t set_size) {
        std::vector<int> reqs; reqs.reserve(num_requests);
        for (size_t i = 0; i < num_requests; ++i) reqs.push_back(static_cast<int>(i % set_size));
        return reqs;
    }
    std::vector<int> generate_scan(size_t num_requests) {
        std::vector<int> reqs; reqs.reserve(num_requests);
        for (size_t i = 0; i < num_requests; ++i) reqs.push_back(static_cast<int>(i));
        return reqs;
    }
    std::vector<int> generate_hot_cold(size_t num_requests, size_t hot_count = 10, size_t cold_count = 1000, double hot_prob = 0.8) {
        std::vector<int> reqs; reqs.reserve(num_requests);
        std::mt19937 gen(42);
        std::bernoulli_distribution is_hot(hot_prob);
        std::uniform_int_distribution<int> hot_dist(1, static_cast<int>(hot_count));
        std::uniform_int_distribution<int> cold_dist(static_cast<int>(hot_count + 1), static_cast<int>(hot_count + cold_count));
        for (size_t i = 0; i < num_requests; ++i) reqs.push_back(is_hot(gen) ? hot_dist(gen) : cold_dist(gen));
        return reqs;
    }
    std::vector<int> generate_working_set(size_t num_requests, size_t set_size = 20, size_t shift_interval = 200) {
        std::vector<int> reqs; reqs.reserve(num_requests);
        std::mt19937 gen(42);
        int offset = 0;
        for (size_t i = 0; i < num_requests; ++i) {
            if (i > 0 && i % shift_interval == 0) offset += static_cast<int>(set_size / 2);
            std::uniform_int_distribution<int> dist(offset, offset + static_cast<int>(set_size) - 1);
            reqs.push_back(dist(gen));
        }
        return reqs;
    }
    std::vector<int> generate_mixed(size_t num_requests) {
        std::vector<int> reqs; reqs.reserve(num_requests);
        std::mt19937 gen(42);
        for (size_t i = 0; i < num_requests; ) {
            int phase = gen() % 3;
            size_t len = std::min<size_t>(100, num_requests - i);
            if (phase == 0) {
                for (size_t j = 0; j < len; ++j) reqs.push_back(static_cast<int>(j % 15));
            } else if (phase == 1) {
                std::uniform_int_distribution<int> dist(1, 100);
                for (size_t j = 0; j < len; ++j) reqs.push_back(dist(gen) <= 80 ? dist(gen) % 5 : dist(gen));
            } else {
                std::uniform_int_distribution<int> dist(1, 300);
                for (size_t j = 0; j < len; ++j) reqs.push_back(dist(gen));
            }
            i += len;
        }
        return reqs;
    }
    std::vector<int> generate_random(size_t num_requests, size_t key_range = 200, uint32_t seed = 42) {
        std::vector<int> reqs; reqs.reserve(num_requests);
        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(1, static_cast<int>(key_range));
        for (size_t i = 0; i < num_requests; ++i) reqs.push_back(dist(gen));
        return reqs;
    }
}

// Запись строки в CSV
void write_csv_row(
    std::ostream& os,
    const std::string& workload,
    const std::string& algo,
    size_t l1_cap,
    size_t l2_cap,
    size_t l3_cap,
    const std::string& mode,
    size_t hits,
    size_t ideal_hits,
    size_t num_reqs)
{
    double hit_ratio = 100.0 * hits / num_reqs;
    double efficiency = 100.0 * hits / ideal_hits;

    os << workload << ","
       << algo << ","
       << l1_cap << ","
       << l2_cap << ","
       << l3_cap << ","
       << mode << ","
       << hits << ","
       << std::fixed << std::setprecision(2)
       << hit_ratio << ","
       << ideal_hits << ","
       << efficiency
       << "\n";
}

// ТЕСТИРОВАНИЕ ОДИНОЧНЫХ КЭШЕЙ
template <typename CacheT>
size_t run_single_cache(size_t capacity, const std::vector<int>& requests) {
    CacheT cache(capacity);
    size_t hits = 0; int dummy_val = 0;
    for (int key : requests) {
        if (cache.lookup(key, dummy_val)) hits++;
        else cache.insert(key, key * 10);
    }
    return hits;
}

void benchmark_single_caches(std::ostream& os, const std::string& wl_name, const std::vector<int>& requests, size_t capacity) {
    size_t ideal_hits = belady_opt_hits(capacity, requests);
    size_t num_reqs = requests.size();

    auto test = [&](const std::string& name, size_t hits) {
        write_csv_row(os, wl_name, name, capacity, 0, 0, "Single", hits, ideal_hits, num_reqs);
    };

    test("LRU", run_single_cache<lru_cache_t<int, int>>(capacity, requests));
    test("2Q",  run_single_cache<two_q_cache_t<int, int>>(capacity, requests));
    test("LFU", run_single_cache<lfu_cache_t<int, int>>(capacity, requests));
    test("LIRS", run_single_cache<lirs_cache_t<int, int>>(capacity, requests));
    test("ARC", run_single_cache<arc_cache_t<int, int>>(capacity, requests));
}

using CacheFactory = std::function <std::unique_ptr<cache_interface<int, int>>(size_t)>;

// ТЕСТИРОВАНИЕ МНОГОУРОВНЕВОГО КЭША
void run_multi2(
    std::ostream& os,
    const std::string& wl,
    const std::string& algo,
    const std::vector<int>& reqs,
    size_t l1,
    size_t l2,
    bool inclusive,
    const CacheFactory& factory1,
    const CacheFactory& factory2) {

    multi_cache_t<int, int> cache;

    cache.add_cache(factory1(l1));
    cache.add_cache(factory2(l2));

    size_t hits = 0;
    int value = 0;

    for (int key : reqs) {
        bool hit;

        if (inclusive) {
            hit = cache.request_inclusive(key, value);
        } 
        else {
            hit = cache.request_exclusive(key, value);
        }

        if (hit) {
            ++hits;
        }
    }

    size_t ideal_capacity;

    if (inclusive) {
        ideal_capacity = l2;
    } 
    else {
        ideal_capacity = l1 + l2;
    }

    size_t ideal_hits = belady_opt_hits(ideal_capacity, reqs);

    std::string mode;

    if (inclusive) {
        mode = "Inclusive";
    } 
    else {
        mode = "Exclusive";
    }

    write_csv_row(
        os,
        wl,
        algo,
        l1,
        l2,
        0,
        mode,
        hits,
        ideal_hits,
        reqs.size()
    );
}

void run_multi3(
    std::ostream& os,
    const std::string& wl,
    const std::string& algo,
    const std::vector<int>& reqs,
    size_t l1,
    size_t l2,
    size_t l3,
    bool inclusive,
    const CacheFactory& factory1,
    const CacheFactory& factory2,
    const CacheFactory& factory3)
{
    multi_cache_t<int, int> cache;

    cache.add_cache(factory1(l1));
    cache.add_cache(factory2(l2));
    cache.add_cache(factory3(l3));

    size_t hits = 0;
    int value = 0;

    for (int key : reqs) {
        bool hit;

        if (inclusive) {
            hit = cache.request_inclusive(key, value);
        } else {
            hit = cache.request_exclusive(key, value);
        }

        if (hit) {
            ++hits;
        }
    }

    size_t ideal_capacity;

    if (inclusive) {
        ideal_capacity = l3;
    } else {
        ideal_capacity = l1 + l2 + l3;
    }

    size_t ideal_hits = belady_opt_hits(ideal_capacity, reqs);

    std::string mode;

    if (inclusive) {
        mode = "Inclusive";
    } else {
        mode = "Exclusive";
    }

    write_csv_row(
        os,
        wl,
        algo,
        l1,
        l2,
        l3,
        mode,
        hits,
        ideal_hits,
        reqs.size()
    );
}

const std::vector<std::pair<std::string, CacheFactory>> cache_factories = {
    {
        "LFU",
        [](size_t capacity) {
            return std::make_unique<lfu_cache_t<int, int>>(capacity);
        }
    },
    {
        "2Q",
        [](size_t capacity) {
            return std::make_unique<two_q_cache_t<int, int>>(capacity);
        }
    },
    {
        "LIRS",
        [](size_t capacity) {
            return std::make_unique<lirs_cache_t<int, int>>(capacity);
        }
    },
    {
        "ARC",
        [](size_t capacity) {
            return std::make_unique<arc_cache_t<int, int>>(capacity);
        }
    }
};

void benchmark_all_multi(
    std::ostream& os,
    const std::string& wl,
    const std::vector<int>& reqs,
    size_t l1,
    size_t l2,
    size_t l3)
{
    for (const auto& cache1 : cache_factories) {
        for (const auto& cache2 : cache_factories) {
            std::string algo =
                cache1.first + "+" + cache2.first;

            run_multi2(
                os,
                wl,
                algo,
                reqs,
                l1,
                l2,
                true,
                cache1.second,
                cache2.second
            );

            run_multi2(
                os,
                wl,
                algo,
                reqs,
                l1,
                l2,
                false,
                cache1.second,
                cache2.second
            );
        }
    }

    for (const auto& cache1 : cache_factories) {
        for (const auto& cache2 : cache_factories) {
            for (const auto& cache3 : cache_factories) {
                std::string algo =
                    cache1.first + "+" +
                    cache2.first + "+" +
                    cache3.first;

                run_multi3(
                    os,
                    wl,
                    algo,
                    reqs,
                    l1,
                    l2,
                    l3,
                    true,
                    cache1.second,
                    cache2.second,
                    cache3.second
                );

                run_multi3(
                    os,
                    wl,
                    algo,
                    reqs,
                    l1,
                    l2,
                    l3,
                    false,
                    cache1.second,
                    cache2.second,
                    cache3.second
                );
            }
        }
    }
}

int main() {
    std::string out_filename = "benchmarks/benchmarks.csv";
    std::ofstream csv_file(out_filename);
    if (!csv_file.is_open()) {
        std::cerr << "Error: Could not open " << out_filename << " for writing.\n";
        return 1;
    }

    // Заголовок CSV
    csv_file << "Workload,Algorithm,L1_Capacity,L2_Capacity,L3_Capacity,"
             << "Mode,Hits,Hit_Ratio_Pct,Ideal_Hits,Efficiency_Pct\n";

    const size_t NUM_REQUESTS = 5000;
    const size_t CACHE_CAPACITY = 30;
    const size_t L1_CAP = 10;
    const size_t L2_CAP = 20;
    const size_t L3_CAP = 30;

    std::cout << "Running benchmarks (CSV output)...\n";

    auto loop_reqs = Workloads::generate_loop(NUM_REQUESTS, 35);
    benchmark_single_caches(csv_file, "LOOP", loop_reqs, CACHE_CAPACITY);
    benchmark_all_multi(csv_file, "LOOP", loop_reqs, L1_CAP, L2_CAP, L3_CAP);

    auto scan_reqs = Workloads::generate_scan(NUM_REQUESTS);
    benchmark_single_caches(csv_file, "SCAN", scan_reqs, CACHE_CAPACITY);
    benchmark_all_multi(csv_file, "SCAN", scan_reqs, L1_CAP, L2_CAP, L3_CAP);

    auto hot_cold_reqs = Workloads::generate_hot_cold(NUM_REQUESTS, 15, 500, 0.85);
    benchmark_single_caches(csv_file, "HOT_COLD", hot_cold_reqs, CACHE_CAPACITY);
    benchmark_all_multi(csv_file, "HOT_COLD", hot_cold_reqs, L1_CAP, L2_CAP, L3_CAP);

    auto ws_reqs = Workloads::generate_working_set(NUM_REQUESTS, 25, 250);
    benchmark_single_caches(csv_file, "WORKING_SET", ws_reqs, CACHE_CAPACITY);
    benchmark_all_multi(csv_file, "WORKING_SET", ws_reqs, L1_CAP, L2_CAP, L3_CAP);

    auto mixed_reqs = Workloads::generate_mixed(NUM_REQUESTS);
    benchmark_single_caches(csv_file, "MIXED", mixed_reqs, CACHE_CAPACITY);
    benchmark_all_multi(csv_file, "MIXED", mixed_reqs, L1_CAP, L2_CAP, L3_CAP);

    auto random_reqs = Workloads::generate_random(NUM_REQUESTS, 150, 42);
    benchmark_single_caches(csv_file, "RANDOM", random_reqs, CACHE_CAPACITY);
    benchmark_all_multi(csv_file, "RANDOM", random_reqs, L1_CAP, L2_CAP, L3_CAP);

    csv_file.close();
    std::cout << "Benchmarks completed successfully! Saved to " << out_filename << "\n";
    return 0;
}
