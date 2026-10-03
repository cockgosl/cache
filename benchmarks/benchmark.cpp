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

void benchmark_single_caches(std::ostream& os, const std::string& workload_name, const std::vector<int>& requests, size_t capacity) {
    size_t ideal_hits = belady_opt_hits(capacity, requests);
    size_t num_reqs = requests.size();

    os << "### Workload: " << workload_name << "\n"
       << "- **Requests**: " << num_reqs << "\n"
       << "- **Capacity**: " << capacity << "\n"
       << "- **Ideal Belady Hits**: " << ideal_hits << " (Hit Ratio: "
       << std::fixed << std::setprecision(2) << (static_cast<double>(ideal_hits) / num_reqs * 100.0) << "%)\n\n";

    os << "| Algorithm | Hits | Hit Ratio (%) | Ideal Hits | Efficiency (%) |\n"
       << "| :--- | :--- | :--- | :--- | :--- |\n";

    auto test_algo = [&](const std::string& name, size_t hits) {
        double hit_ratio = (static_cast<double>(hits) / num_reqs) * 100.0;
        double efficiency = ideal_hits > 0 ? (static_cast<double>(hits) / ideal_hits) * 100.0 : 0.0;

        os << "| " << name << " | " << hits << " | "
           << std::fixed << std::setprecision(2) << hit_ratio << " | "
           << ideal_hits << " | "
           << std::fixed << std::setprecision(2) << efficiency << " |\n";
    };

    test_algo("LRU", run_single_cache<lru_cache_t<int, int>>(capacity, requests));
    test_algo("2Q",  run_single_cache<two_q_cache_t<int, int>>(capacity, requests));
    test_algo("LFU", run_single_cache<lfu_cache_t<int, int>>(capacity, requests));
    test_algo("LIRS", run_single_cache<lirs_cache_t<int, int>>(capacity, requests));
    test_algo("ARC", run_single_cache<arc_cache_t<int, int>>(capacity, requests));
    os << "\n";
}

// ============================================================================
// 4. ТЕСТИРОВАНИЕ МНОГОУРОВНЕВОГО КЭША
// ============================================================================
void benchmark_multi_cache(std::ostream& os, const std::string& workload_name, const std::vector<int>& requests, size_t l1_cap, size_t l2_cap) {
    size_t total_cap = l1_cap + l2_cap;
    size_t ideal_hits = belady_opt_hits(total_cap, requests);
    size_t num_reqs = requests.size();

    os << "#### Multi-Level Cache (LRU+LRU) | " << workload_name << "\n"
       << "- **L1 Capacity**: " << l1_cap << ", **L2 Capacity**: " << l2_cap << " (Total: " << total_cap << ")\n\n";

    auto run_multi = [&](bool inclusive) {
        multi_cache_t<int, int> mc;
        mc.add_cache<lru_cache_t<int, int>>(l1_cap);
        mc.add_cache<lru_cache_t<int, int>>(l2_cap);

        size_t total_hits = 0;
        int dummy = 0;
        for (int key : requests) {
            if (inclusive ? mc.request_inclusive(key, dummy) : mc.request_exclusive(key, dummy)) {
                total_hits++;
            }
        }
        size_t mem_accesses = num_reqs - total_hits;
        double hit_ratio = (static_cast<double>(total_hits) / num_reqs) * 100.0;
        double efficiency = ideal_hits > 0 ? (static_cast<double>(total_hits) / ideal_hits) * 100.0 : 0.0;

        std::string mode = inclusive ? "INCLUSIVE" : "EXCLUSIVE";
        os << "**[" << mode << "]**\n"
           << "- Total Hits: " << total_hits << "\n"
           << "- Hit Ratio: " << std::fixed << std::setprecision(2) << hit_ratio << "%\n"
           << "- Efficiency: " << std::fixed << std::setprecision(2) << efficiency << "%\n"
           << "- Memory Accesses: " << mem_accesses << "\n\n";
    };

    run_multi(true);
    run_multi(false);
    os << "---\n\n";
}

// ============================================================================
// MAIN
// ============================================================================
int main() {
    std::string out_filename = "benchmarks/benchmarks.md";
    std::ofstream md_file(out_filename);

    if (!md_file.is_open()) {
        std::cerr << "Error: Could not open " << out_filename << " for writing.\n";
        return 1;
    }

    const size_t NUM_REQUESTS = 5000;
    const size_t CACHE_CAPACITY = 30;

    std::cout << "Running benchmarks and writing results to " << out_filename << "...\n";

    md_file << "# Cache Algorithms Benchmark Results\n\n";

    std::cout << "Testing LOOP..." << std::endl;
    auto loop_reqs = Workloads::generate_loop(NUM_REQUESTS, 35);
    benchmark_single_caches(md_file, "LOOP", loop_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(md_file, "LOOP", loop_reqs, 10, 20);

    std::cout << "Testing SCAN..." << std::endl;
    auto scan_reqs = Workloads::generate_scan(NUM_REQUESTS);
    benchmark_single_caches(md_file, "SCAN", scan_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(md_file, "SCAN", scan_reqs, 10, 20);

    std::cout << "Testing HOT/COLD..." << std::endl;
    auto hot_cold_reqs = Workloads::generate_hot_cold(NUM_REQUESTS, 15, 500, 0.85);
    benchmark_single_caches(md_file, "HOT/COLD", hot_cold_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(md_file, "HOT/COLD", hot_cold_reqs, 10, 20);

    std::cout << "Testing WORKING_SET..." << std::endl;
    auto ws_reqs = Workloads::generate_working_set(NUM_REQUESTS, 25, 250);
    benchmark_single_caches(md_file, "WORKING_SET", ws_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(md_file, "WORKING_SET", ws_reqs, 10, 20);

    std::cout << "Testing MIXED..." << std::endl;
    auto mixed_reqs = Workloads::generate_mixed(NUM_REQUESTS);
    benchmark_single_caches(md_file, "MIXED", mixed_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(md_file, "MIXED", mixed_reqs, 10, 20);

    std::cout << "Testing RANDOM..." << std::endl;
    auto random_reqs = Workloads::generate_random(NUM_REQUESTS, 150, 42);
    benchmark_single_caches(md_file, "RANDOM", random_reqs, CACHE_CAPACITY);
    benchmark_multi_cache(md_file, "RANDOM", random_reqs, 10, 20);

    std::cout << "Benchmarks completed successfully!\n";
    md_file.close();

    return 0;
}
