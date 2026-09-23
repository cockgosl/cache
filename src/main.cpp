#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>

#include "cache.hpp"

struct BenchmarkResult {
    std::string cache_type;   // Название кеша (например, "LRU Cache")
    size_t capacity;          // Ёмкость кеша (максимальное количество элементов)
    size_t total_requests;    // Всего запросов
    size_t hits;              // Количество попаданий (Hit)
    size_t misses;            // Количество промахов (Miss)
    double hit_rate;          // Эффективность в процентах (Hits / Total * 100%)
    double duration_ms;       // Время выполнения теста в миллисекундах
};

// Генерация последовательности запросов (Zipfian / Hotspot: 80% запросов к 20% элементов)
std::vector<int> generate_zipf_workload(size_t num_requests, int num_elements) {
    std::vector<int> requests;
    requests.reserve(num_requests);
    std::mt19937 gen(42); // Фиксированный seed для воспроизводимости

    std::discrete_distribution<> dist({80, 5, 5, 2, 2, 2, 1, 1, 1, 1});

    for (size_t i = 0; i < num_requests; ++i) {
        int key = dist(gen) * (num_elements / 10) + (gen() % (num_elements / 10));
        requests.push_back(key);
    }
    return requests;
}

// Запуск бенчмарка для одного кеша
template <typename CacheT>
BenchmarkResult run_benchmark(const std::string& name, size_t capacity, const std::vector<int>& workload) {
    CacheT cache(capacity);
    size_t hits = 0;
    size_t misses = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int key : workload) {
    if (cache.lookup(key)) {
        hits++;
    } else {
        misses++;
        slow_get_page(key);
        cache.insert(key);
    }
}

    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double, std::milli>(end - start).count();

    double hit_rate = (double)hits / workload.size() * 100.0;

    return {name, capacity, workload.size(), hits, misses, hit_rate, duration};
}

// Запуск всех вариантов кеша и сохранение результатов в Markdown-файл
void run_all_tests_and_save(const std::string& output_filename) {
    std::cout << "[TEST] Running cache benchmarks..." << std::endl;

    const size_t NUM_REQUESTS = 100'000;
    const int NUM_ELEMENTS = 2'000;
    auto workload = generate_zipf_workload(NUM_REQUESTS, NUM_ELEMENTS);

    std::vector<size_t> capacities = {100, 250, 500, 1000};
    std::vector<BenchmarkResult> results;

    for (size_t cap : capacities) {
        // Указываем lru_cache_t<int> с одним параметром шаблона
        results.push_back(run_benchmark<lru_cache_t<int>>("LRU Cache", cap, workload));

        // Если у вас реализованы другие кеши, добавьте их по аналогии:
        // results.push_back(run_benchmark<lfu_cache_t<int>>("LFU Cache", cap, workload));
        // results.push_back(run_benchmark<two_q_cache_t<int>>("2Q Cache", cap, workload));
    }

    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file " << output_filename << " for writing.\n";
        return;
    }

    outfile << "# Результаты бенчмарка кеширования\n\n";
    outfile << "- **Количество запросов:** " << NUM_REQUESTS << "\n";
    outfile << "- **Диапазон ключей:** " << NUM_ELEMENTS << "\n";
    outfile << "- **Паттерн нагрузки:** Zipfian (80/20 Hotspot)\n\n";

    outfile << "| Алгоритм | Ёмкость кеша | Попаданий (Hits) | Промахов (Misses) | Hit Rate (%) | Время (мс) |\n";
    outfile << "| :--- | :---: | :---: | :---: | :---: | :---: |\n";

    for (const auto& res : results) {
        outfile << "| " << res.cache_type
                << " | " << res.capacity
                << " | " << res.hits
                << " | " << res.misses
                << " | " << std::fixed << std::setprecision(2) << res.hit_rate << "%"
                << " | " << std::fixed << std::setprecision(2) << res.duration_ms << " ms |\n";
    }

    outfile.close();
    std::cout << "[TEST] Benchmarks finished successfully! Results saved to '" << output_filename << "'.\n";
}

int main(int argc, char* argv[]) {
    // Проверка флага -test
    if (argc > 1 && std::string(argv[1]) == "-test") {
        run_all_tests_and_save("benchmark_results.md");
        return 0;
    }

    // Стандартный режим работы (без флага -test)
    std::cout << "Normal execution mode. Use '-test' flag to run benchmarks.\n";
    return 0;
}
