#include "../include/cache.hpp"
#include "../include/cache_api.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <random>
#include <iomanip>
#include <string>
#include <algorithm>

// ============================================================================
// 1. ИДЕАЛЬНЫЙ АЛГОРИТМ БЕЛАДИ (Belady's OPT)
// ============================================================================
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
                if (victim_key != -1) {
                    cache.erase(victim_key);
                }
            }
            cache[key] = next_occ;
        }
    }
    return hits;
}

// ============================================================================
// 2. ГЕНЕРАТОРЫ НАГРУЗОК (WORKLOADS)
// ============================================================================
namespace Workloads {

std::vector<int> generate_loop(size_t num_requests, size_t set_size) {
    std::vector<int> reqs;
    reqs.reserve(num_requests);
    for (size_t i = 0; i < num_requests; ++i) {
        reqs.push_back(static_cast<int>(i % set_size));
    }
    return reqs;
}

std::vector<int> generate_scan(size_t num_requests) {
    std::vector<int> reqs;
    reqs.reserve(num_requests);
    for (size_t i = 0; i < num_requests; ++i) {
        reqs.push_back(static_cast<int>(i));
    }
    return reqs;
}

std::vector<int> generate_hot_cold(size_t num_requests, size_t hot_count = 10, size_t cold_count = 1000, double hot_prob = 0.8) {
    std::vector<int> reqs;
    reqs.reserve(num_requests);
    std::mt19937 gen(42);
    std::bernoulli_distribution is_hot(hot_prob);
    std::uniform_int_distribution<int> hot_dist(1, static_cast<int>(hot_count));
    std::uniform_int_distribution<int> cold_dist(static_cast<int>(hot_count + 1), static_cast<int>(hot_count + cold_count));

    for (size_t i = 0; i < num_requests; ++i) {
        reqs.push_back(is_hot(gen) ? hot_dist(gen) : cold_dist(gen));
    }
    return reqs;
}

std::vector<int> generate_working_set(size_t num_requests, size_t set_size = 20, size_t shift_interval = 200) {
    std::vector<int> reqs;
    reqs.reserve(num_requests);
    std::mt19937 gen(42);

    int offset = 0;
    for (size_t i = 0; i < num_requests; ++i) {
        if (i > 0 && i % shift_interval == 0) {
            offset += static_cast<int>(set_size / 2);
        }
        std::uniform_int_distribution<int> dist(offset, offset + static_cast<int>(set_size) - 1);
        reqs.push_back(dist(gen));
    }
    return reqs;
}

std::vector<int> generate_mixed(size_t num_requests) {
    std::vector<int> reqs;
    reqs.reserve(num_requests);
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
    std::vector<int> reqs;
    reqs.reserve(num_requests);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(1, static_cast<int>(key_range));

    for (size_t i = 0; i < num_requests; ++i) {
        reqs.push_back(dist(gen));
    }
    return reqs;
}

} // namespace Workloads

// ============================================================================
// 3. ТЕСТИРОВАНИЕ ОДИНОЧНЫХ КЭШЕЙ
// ============================================================================
template <typename CacheT>
size_t run_single_cache(size_t capacity, const std::vector<int>& requests) {
    CacheT cache(capacity);
    size_t hits = 0;
    int dummy_val = 0;

    for (int key : requests) {
        if (cache.lookup(key, dummy_val)) {
            hits++;
        } else {
            cache.insert(key, key * 10);
        }
    }
    return hits;
}

void benchmark_single_caches(std::ostream& csv, const std::string& workload_name, const std::vector<int>& requests, size_t capacity) {
    size_t ideal_hits = belady_opt_hits(capacity, requests);
    size_t num_reqs = requests.size();

    auto test_algo = [&](const std::string& name, size_t hits) {
        double hit_ratio = (static_cast<double>(hits) / num_reqs) * 100.0;
        double efficiency = ideal_hits > 0 ? (static_cast<double>(hits) / ideal_hits) * 100.0 : 0.0;
        size_t mem_accesses = num_reqs - hits;

        csv << workload_name << ",Single," << name << "," << capacity << ","
            << hits << "," << std::fixed << std::setprecision(2) << hit_ratio << ","
            << ideal_hits << "," << std::fixed << std::setprecision(2) << efficiency << ","
            << mem_accesses << "\n";
    };

    test_algo("LRU", run_single_cache<lru_cache_t<int, int>>(capacity, requests));
    test_algo("2Q",  run_single_cache<two_q_cache_t<int, int>>(capacity, requests));
    test_algo("LFU", run_single_cache<lfu_cache_t<int, int>>(capacity, requests));
    test_algo("LIRS", run_single_cache<lirs_cache_t<int, int>>(capacity, requests));
    test_algo("ARC", run_single_cache<arc_cache_t<int, int>>(capacity, requests));
}

// ============================================================================
// 4. ТЕСТИРОВАНИЕ МНОГОУРОВНЕВОГО КЭША (Исправлен ОРТ и добавлены алгоритмы)
// ============================================================================
void benchmark_multi_cache(std::ostream& csv, const std::string& workload_name, const std::vector<int>& requests, size_t l1_cap, size_t l2_cap) {
    // В inclusive режиме элементы дублируются, максимальное количество уникальных страниц = max(L1, L2)
    size_t incl_cap = std::max(l1_cap, l2_cap);
    // В exclusive режиме элементы не дублируются, уникальных страниц = L1 + L2
    size_t excl_cap = l1_cap + l2_cap;

    size_t ideal_hits_incl = belady_opt_hits(incl_cap, requests);
    size_t ideal_hits_excl = belady_opt_hits(excl_cap, requests);
    size_t num_reqs = requests.size();

    // Лямбда для прогона конкретной связки
    auto test_combo = [&](const std::string& algo_name, auto init_mc) {
        // [INCLUSIVE]
        {
            auto mc = init_mc();
            size_t total_hits = 0; int dummy = 0;
            for (int key : requests) if (mc.request_inclusive(key, dummy)) total_hits++;

            double hit_ratio = (static_cast<double>(total_hits) / num_reqs) * 100.0;
            double efficiency = ideal_hits_incl > 0 ? (static_cast<double>(total_hits) / ideal_hits_incl) * 100.0 : 0.0;
            size_t mem_accesses = num_reqs - total_hits;

            csv << workload_name << ",Multi_Inclusive," << algo_name << "," << incl_cap << ","
                << total_hits << "," << std::fixed << std::setprecision(2) << hit_ratio << ","
                << ideal_hits_incl << "," << std::fixed << std::setprecision(2) << efficiency << ","
                << mem_accesses << "\n";
        }
        // [EXCLUSIVE]
        {
            auto mc = init_mc();
            size_t total_hits = 0; int dummy = 0;
            for (int key : requests) if (mc.request_exclusive(key, dummy)) total_hits++;

            double hit_ratio = (static_cast<double>(total_hits) / num_reqs) * 100.0;
            double efficiency = ideal_hits_excl > 0 ? (static_cast<double>(total_hits) / ideal_hits_excl) * 100.0 : 0.0;
            size_t mem_accesses = num_reqs - total_hits;

            csv << workload_name << ",Multi_Exclusive," << algo_name << "," << excl_cap << ","
                << total_hits << "," << std::fixed << std::setprecision(2) << hit_ratio << ","
                << ideal_hits_excl << "," << std::fixed << std::setprecision(2) << efficiency << ","
                << mem_accesses << "\n";
        }
    };

    // Тестируем различные комбинации (L1 + L2)
    test_combo("LRU+LRU", [&](){ multi_cache_t<int, int> mc; mc.add_cache<lru_cache_t<int, int>>(l1_cap); mc.add_cache<lru_cache_t<int, int>>(l2_cap); return mc; });
    test_combo("2Q+LRU",  [&](){ multi_cache_t<int, int> mc; mc.add_cache<two_q_cache_t<int, int>>(l1_cap); mc.add_cache<lru_cache_t<int, int>>(l2_cap); return mc; });
    test_combo("LFU+LRU", [&](){ multi_cache_t<int, int> mc; mc.add_cache<lfu_cache_t<int, int>>(l1_cap); mc.add_cache<lru_cache_t<int, int>>(l2_cap); return mc; });
    test_combo("LIRS+LRU",[&](){ multi_cache_t<int, int> mc; mc.add_cache<lirs_cache_t<int, int>>(l1_cap); mc.add_cache<lru_cache_t<int, int>>(l2_cap); return mc; });
    test_combo("ARC+LRU", [&](){ multi_cache_t<int, int> mc; mc.add_cache<arc_cache_t<int, int>>(l1_cap); mc.add_cache<lru_cache_t<int, int>>(l2_cap); return mc; });
}

// ============================================================================
// MAIN
// ============================================================================
int main() {
    std::string out_filename = "benchmarks/benchmarks.csv";
    std::ofstream csv_file(out_filename);

    if (!csv_file.is_open()) {
        std::cerr << "Error: Could not open " << out_filename << " for writing.\n";
        return 1;
    }

    const size_t NUM_REQUESTS = 5000;
    const size_t CACHE_CAPACITY = 30;

    std::cout << "Running benchmarks and writing results to " << out_filename << "...\n";

    // Записываем заголовок CSV
    csv_file << "Workload,Architecture,Algorithm,Capacity,Hits,Hit_Ratio_%,Ideal_Hits,Efficiency_%,Memory_Accesses\n";

    std::cout << "Testing LOOP..." << std::endl;
    auto loop_reqs = Workloads::generate_loop(NUM_REQUESTS, 35);
    benchmark_single_caches(csv_file, "LOOP", loop_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(csv_file, "LOOP", loop_reqs, 10, 20);

    std::cout << "Testing SCAN..." << std::endl;
    auto scan_reqs = Workloads::generate_scan(NUM_REQUESTS);
    benchmark_single_caches(csv_file, "SCAN", scan_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(csv_file, "SCAN", scan_reqs, 10, 20);

    std::cout << "Testing HOT/COLD..." << std::endl;
    auto hot_cold_reqs = Workloads::generate_hot_cold(NUM_REQUESTS, 15, 500, 0.85);
    benchmark_single_caches(csv_file, "HOT/COLD", hot_cold_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(csv_file, "HOT/COLD", hot_cold_reqs, 10, 20);

    std::cout << "Testing WORKING_SET..." << std::endl;
    auto ws_reqs = Workloads::generate_working_set(NUM_REQUESTS, 25, 250);
    benchmark_single_caches(csv_file, "WORKING_SET", ws_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(csv_file, "WORKING_SET", ws_reqs, 10, 20);

    std::cout << "Testing MIXED..." << std::endl;
    auto mixed_reqs = Workloads::generate_mixed(NUM_REQUESTS);
    benchmark_single_caches(csv_file, "MIXED", mixed_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(csv_file, "MIXED", mixed_reqs, 10, 20);

    std::cout << "Testing RANDOM..." << std::endl;
    auto random_reqs = Workloads::generate_random(NUM_REQUESTS, 150, 42);
    benchmark_single_caches(csv_file, "RANDOM", random_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(csv_file, "RANDOM", random_reqs, 10, 20);

    std::cout << "Benchmarks completed successfully!\n";
    csv_file.close();

    return 0;
}
