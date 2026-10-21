#include "interface/cache_api.hpp"
#include "cache_algo/lru_cache.hpp"
#include "cache_algo/lfu_cache.hpp"
#include "cache_algo/two_q_cache.hpp"
#include "cache_algo/arc_cache.hpp"
#include "cache_algo/lirs_cache.hpp"

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
#include <ranges>

size_t belady_opt_hits(size_t capacity, const std::vector<int>& requests) {
    if (capacity == 0 || requests.empty()) return 0;

    size_t n = requests.size(); //the amount of requests
    std::vector<size_t> next_pos(n, n + 1);
    std::unordered_map<int, size_t> last_seen;

    for (auto i : std::views::iota(0uz , n) | std::views::reverse) {
        auto it = last_seen.find(requests[i]);
        if (it != last_seen.end()) {
            next_pos[i] = it->second;
        }
        last_seen[requests[i]] = i;
    }

    std::unordered_map<int, size_t> cache;
    size_t hits = 0;

    for (auto [key, next_occ] : std::views::zip(requests, next_pos)) {

        auto it = cache.find(key);
        if (it != cache.end()) {
            hits++;
            it->second = next_occ;
            continue;
        } 
        if (cache.size() >= capacity) {
            using Pair = decltype(cache)::value_type;
            auto victim_it = std::ranges::max_element(cache, {}, &Pair::second);
            cache.erase(victim_it);

        }
        cache[key] = next_occ;
    }
    return hits;
}

// Workload generator 
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
            if (is_hot(gen)) {
                reqs.push_back(hot_dist(gen));
            }
            else {
                reqs.push_back(cold_dist(gen));
            }
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
                for (size_t j = 0; j < len; ++j) {
                    reqs.push_back(static_cast<int>(j % 15));
                }
            } 
            else if (phase == 1) {
                std::uniform_int_distribution<int> dist(1, 100);
                for (size_t j = 0; j < len; ++j) {
                    if (dist(gen) <= 80) {
                        reqs.push_back(dist(gen) % 5);
                    }
                    else {
                        reqs.push_back(dist(gen));
                    }
                }
            } 
            else {
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
}

struct BenchmarkConfig {
    size_t num_requests = 5000;
    size_t cache_capacity = 30;
    size_t l1_capacity = 10;
    size_t l2_capacity = 20;
    size_t l3_capacity = 30;
};

struct BenchmarkResult {
    size_t hits = 0;
    size_t ideal_hits = 0;
    size_t l1_hits = 0;
    size_t l2_hits = 0;
    size_t l3_hits = 0;
};

struct BenchmarkMetadata {
    std::string workload;
    std::string algorithm;
    std::string mode;
    size_t levels = 1;
};

// write a row in CSV file

void write_csv(
    std::ostream& os,
    const BenchmarkConfig& config,
    const BenchmarkMetadata& metadata,
    const BenchmarkResult& result)
{
    const double hit_ratio = config.num_requests > 0  ? 100.0 * result.hits / config.num_requests : 0.0;

    const double efficiency = result.ideal_hits > 0 ? 100.0 * result.hits / result.ideal_hits : 0.0;

    const size_t l1_capacity = metadata.levels == 1 ? config.cache_capacity : config.l1_capacity;

    const size_t l2_capacity = metadata.levels >= 2 ? config.l2_capacity : 0;

    const size_t l3_capacity = metadata.levels >= 3 ? config.l3_capacity : 0;
    os << metadata.workload << ','
       << metadata.algorithm << ','
       << l1_capacity << ','
       << l2_capacity << ','
       << l3_capacity << ','
       << metadata.mode << ','
       << result.hits << ','
       << std::fixed << std::setprecision(2)
       << hit_ratio << ','
       << result.ideal_hits << ','
       << efficiency << ','
       << result.l1_hits << ','
       << result.l2_hits << ','
       << result.l3_hits << '\n';
}

// one level run 
template <typename CacheT>
size_t run_single_cache(size_t capacity, const std::vector<int>& requests) {
    CacheT cache(capacity);
    size_t hits = 0;
    int dummy_val = 0;

    for (int key : requests) {
        if (cache.lookup(key, dummy_val)) hits++;
        else cache.insert(key, key * 10);
    }
    return hits;
}

void benchmark_single_caches(
    std::ostream& os,
    const std::string& wl_name,
    const std::vector<int>& requests,
    const BenchmarkConfig& config)
{
    const size_t capacity = config.cache_capacity;
    const size_t ideal_hits = belady_opt_hits(capacity, requests);

    auto test = [&](const std::string& name, size_t hits) {
        BenchmarkMetadata metadata{
            .workload = wl_name,
            .algorithm = name,
            .mode = "Single",
            .levels = 1
        };

        BenchmarkResult result{
            .hits = hits,
            .ideal_hits = ideal_hits
        };

        write_csv(os, config, metadata, result);
    };

    test("LRU", run_single_cache<lru_cache_t<int, int>>(
        capacity, requests));
    test("2Q", run_single_cache<two_q_cache_t<int, int>>(
        capacity, requests));
    test("LFU", run_single_cache<lfu_cache_t<int, int>>(
        capacity, requests));
    test("LIRS", run_single_cache<lirs_cache_t<int, int>>(
        capacity, requests));
    test("ARC", run_single_cache<arc_cache_t<int, int>>(
        capacity, requests));
}

using CacheFactory = std::function <std::unique_ptr<cache_interface<int, int>>(size_t)>;

// multi level run 
void run_multi2(
    std::ostream& os,
    const std::string& wl,
    const std::string& algo,
    const std::vector<int>& reqs,
    const BenchmarkConfig& config,
    bool inclusive,
    const CacheFactory& factory1,
    const CacheFactory& factory2)
{
    multi_cache_t<int, int> cache;

    cache.add_cache(factory1(config.l1_capacity));
    cache.add_cache(factory2(config.l2_capacity));

    size_t hits = 0;
    int value = 0;

    for (int key : reqs) {
        const bool hit = inclusive
            ? cache.request_inclusive(key, value)
            : cache.request_exclusive(key, value);

        if (hit) {
            ++hits;
        }
    }

    const size_t ideal_capacity = inclusive
        ? config.l2_capacity
        : config.l1_capacity + config.l2_capacity;

    const size_t ideal_hits = belady_opt_hits(ideal_capacity, reqs);

    BenchmarkMetadata metadata{
        .workload = wl,
        .algorithm = algo,
        .mode = inclusive ? "Inclusive" : "Exclusive",
        .levels = 2
    };

    BenchmarkResult result{
        .hits = hits,
        .ideal_hits = ideal_hits,
        .l1_hits = cache.hits(0),
        .l2_hits = cache.hits(1)
    };

    write_csv(os, config, metadata, result);
}

void run_multi3(
    std::ostream& os,
    const std::string& wl,
    const std::string& algo,
    const std::vector<int>& reqs,
    const BenchmarkConfig& config,
    bool inclusive,
    const CacheFactory& factory1,
    const CacheFactory& factory2,
    const CacheFactory& factory3)
{
    multi_cache_t<int, int> cache;

    cache.add_cache(factory1(config.l1_capacity));
    cache.add_cache(factory2(config.l2_capacity));
    cache.add_cache(factory3(config.l3_capacity));

    size_t hits = 0;
    int value = 0;

    for (int key : reqs) {
        const bool hit = inclusive
            ? cache.request_inclusive(key, value)
            : cache.request_exclusive(key, value);

        if (hit) {
            ++hits;
        }
    }

    const size_t ideal_capacity = inclusive
        ? config.l3_capacity
        : config.l1_capacity + config.l2_capacity + config.l3_capacity;

    const size_t ideal_hits = belady_opt_hits(ideal_capacity, reqs);

    BenchmarkMetadata metadata{
        .workload = wl,
        .algorithm = algo,
        .mode = inclusive ? "Inclusive" : "Exclusive",
        .levels = 3
    };

    BenchmarkResult result{
        .hits = hits,
        .ideal_hits = ideal_hits,
        .l1_hits = cache.hits(0),
        .l2_hits = cache.hits(1),
        .l3_hits = cache.hits(2)
    };

    write_csv(os, config, metadata, result);
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
    const BenchmarkConfig& config)
{
    for (const auto& cache1 : cache_factories) {
        for (const auto& cache2 : cache_factories) {
            const std::string algo =
                cache1.first + "+" + cache2.first;

            run_multi2(os, wl, algo, reqs, config, true,
                       cache1.second, cache2.second);

            run_multi2(os, wl, algo, reqs, config, false,
                       cache1.second, cache2.second);
        }
    }

    for (const auto& cache1 : cache_factories) {
        for (const auto& cache2 : cache_factories) {
            for (const auto& cache3 : cache_factories) {
                const std::string algo =
                    cache1.first + "+" + cache2.first + "+" +
                    cache3.first;

                run_multi3(os, wl, algo, reqs, config, true,
                           cache1.second, cache2.second, cache3.second);

                run_multi3(os, wl, algo, reqs, config, false,
                           cache1.second, cache2.second, cache3.second);
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

    // head of the CSV
    csv_file << "Workload,Algorithm,L1_Capacity,L2_Capacity,L3_Capacity,"
                "Mode,Hits,Hit_Ratio_Pct,Ideal_Hits,Efficiency_Pct,"
                "L1_Hits,L2_Hits,L3_Hits\n";   
    const BenchmarkConfig config{
        .num_requests = 5000,
        .cache_capacity = 30,
        .l1_capacity = 10,
        .l2_capacity = 20,
        .l3_capacity = 30
    };

    std::cout << "Running benchmarks (CSV output)...\n";

    auto loop_reqs = Workloads::generate_loop(config.num_requests, 35);
    benchmark_single_caches(csv_file, "LOOP", loop_reqs, config);
    benchmark_all_multi(csv_file, "LOOP", loop_reqs, config);

    auto scan_reqs = Workloads::generate_scan(config.num_requests);
    benchmark_single_caches(csv_file, "SCAN", scan_reqs, config);
    benchmark_all_multi(csv_file, "SCAN", scan_reqs, config);

    auto hot_cold_reqs = Workloads::generate_hot_cold(config.num_requests, 15, 500, 0.85);
    benchmark_single_caches(csv_file, "HOT_COLD", hot_cold_reqs, config);
    benchmark_all_multi(csv_file, "HOT_COLD", hot_cold_reqs, config);

    auto ws_reqs = Workloads::generate_working_set(config.num_requests, 25, 250);
    benchmark_single_caches(csv_file, "WORKING_SET", ws_reqs, config);
    benchmark_all_multi(csv_file, "WORKING_SET", ws_reqs, config);

    auto mixed_reqs = Workloads::generate_mixed(config.num_requests);
    benchmark_single_caches(csv_file, "MIXED", mixed_reqs, config);
    benchmark_all_multi(csv_file, "MIXED", mixed_reqs, config);

    auto random_reqs = Workloads::generate_random(config.num_requests, 150, 42);
    benchmark_single_caches(csv_file, "RANDOM", random_reqs, config);
    benchmark_all_multi(csv_file, "RANDOM", random_reqs, config);

    csv_file.close();
    std::cout << "Benchmarks completed successfully! Saved to " << out_filename << "\n";
    return 0;
}
