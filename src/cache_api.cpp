#include "cache.hpp"
#include "cache_api.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <iomanip>
#include <string>
#include <iostream>

void run_simulation(multi_cache_t<int, int>& cache, std::istream& is) {
    int page_key = 0;
    int value = 0;
    while (is >> page_key) {
        value = cache.slow_get_page(page_key);
        cache.request_inclusive(page_key, value);
    }
    if (!is.eof() && is.fail()) {
        std::cerr << "Ошибка: встречен некорректный символ во входном файле.\n";
        exit(1);
    }
}

// Идеальный кэш Белади (для подсчета теоретического максимума hits)
size_t ideal_cache_hits(const std::vector<int>& requests, size_t capacity) {
    std::unordered_set<int> cache;
    size_t hits = 0;

    for (size_t i = 0; i < requests.size(); ++i) {
        if (cache.find(requests[i]) != cache.end()) {
            hits++;
        } else {
            if (cache.size() == capacity) {
                int furthest_key = -1;
                size_t furthest_idx = 0;

                for (int key : cache) {
                    size_t next_idx = i + 1;
                    while (next_idx < requests.size() && requests[next_idx] != key) {
                        next_idx++;
                    }
                    if (next_idx > furthest_idx) {
                        furthest_idx = next_idx;
                        furthest_key = key;
                    }
                }
                cache.erase(furthest_key);
            }
            cache.insert(requests[i]);
        }
    }
    return hits;
}

// Обёртка тестирования L1 для Key-Value структуры
template <typename Cache1>
size_t test_cache_l1(const std::vector<int>& reqs, size_t cap) {
    multi_cache_t<int, int> cache;
    cache.add_cache<Cache1>(cap);

    size_t total_hits = 0;
    int val = 0;
    for (int r : reqs) {
        if (cache.request_inclusive(r, val)) total_hits++;
    }
    return total_hits;
}

// Обёртка тестирования L1 + L2 (включая смешанные конфигурации)
template <typename Cache1, typename Cache2>
size_t test_cache_l2(const std::vector<int>& reqs, size_t cap1, size_t cap2) {
    multi_cache_t<int, int> cache;
    cache.add_cache<Cache1>(cap1);
    cache.add_cache<Cache2>(cap2);

    size_t total_hits = 0;
    int val = 0;
    for (int r : reqs) {
        if (cache.request_inclusive(r, val)) total_hits++;
    }
    return total_hits;
}


struct RowResult {
    std::string name;
    std::vector<size_t> hits;
};

void run_unit_tests(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: Не удалось открыть файл тестов '" << filename << "'\n";
        return;
    }

    std::vector<std::vector<int>> test_data;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::vector<int> reqs;
        int val;
        while (iss >> val) reqs.push_back(val);
        if (!reqs.empty()) test_data.push_back(reqs);
    }

    if (test_data.empty()) {
        std::cerr << "Внимание: Файл тестов пуст или имеет неверный формат.\n";
        return;
    }

    const size_t L1_SIZE = 20;
    const size_t L2_SIZE = 50;

    // Алиасы типов для ухода от проблем с запятыми в макросах
    using LRUCache  = lru_cache_t<int, int>;
    using LFUCache  = lfu_cache_t<int, int>;
    using TwoQCache = two_q_cache_t<int, int>;
    using ARCCache  = arc_cache_t<int, int>;
    using LIRSCache = lirs_cache_t<int, int>;

    std::vector<RowResult> results_l1;
    std::vector<RowResult> results_l2;
    std::vector<size_t> ideal_l1_hits;
    std::vector<size_t> ideal_l2_hits;

    #define RUN_L1(NAME, CACHE) \
        { RowResult r; r.name = NAME; \
          for (const auto& reqs : test_data) r.hits.push_back(test_cache_l1<CACHE>(reqs, L1_SIZE)); \
          results_l1.push_back(r); }

    #define RUN_L2(NAME, CACHE1, CACHE2) \
        { RowResult r; r.name = NAME; \
          for (const auto& reqs : test_data) r.hits.push_back(test_cache_l2<CACHE1, CACHE2>(reqs, L1_SIZE, L2_SIZE)); \
          results_l2.push_back(r); }

    std::cout << "[TEST] Выполнение тестов L1...\n";
    RUN_L1("LRU",  LRUCache);
    RUN_L1("LFU",  LFUCache);
    RUN_L1("2Q",   TwoQCache);
    RUN_L1("ARC",  ARCCache);
    RUN_L1("LIRS", LIRSCache);

    std::cout << "[TEST] Выполнение тестов L1+L2 (однородные и смешанные)...\n";
    // Однородные
    RUN_L2("LRU + LRU",   LRUCache,  LRUCache);
    RUN_L2("LFU + LFU",   LFUCache,  LFUCache);
    RUN_L2("2Q + 2Q",     TwoQCache, TwoQCache);
    RUN_L2("ARC + ARC",   ARCCache,  ARCCache);
    RUN_L2("LIRS + LIRS", LIRSCache, LIRSCache);

    // Смешанные / Гибридные
    RUN_L2("LRU + ARC",   LRUCache,  ARCCache);
    RUN_L2("LRU + LIRS",  LRUCache,  LIRSCache);
    RUN_L2("LRU + 2Q",    LRUCache,  TwoQCache);
    RUN_L2("LFU + ARC",   LFUCache,  ARCCache);
    RUN_L2("ARC + LRU",   ARCCache,  LRUCache);
    RUN_L2("LIRS + LRU",  LIRSCache, LRUCache);

    #undef RUN_L1
    #undef RUN_L2

    std::cout << "[TEST] Вычисление идеального кэша Белади...\n";
    for (const auto& reqs : test_data) {
        ideal_l1_hits.push_back(ideal_cache_hits(reqs, L1_SIZE));
        ideal_l2_hits.push_back(ideal_cache_hits(reqs, L1_SIZE + L2_SIZE));
    }

    // Генерация таблицы Markdown
    std::ofstream md("cache_results.md");
    md << "# Результаты тестирования кэшей (Вертикальная таблица)\n\n";
    md << "**Параметры:** Ёмкость L1 = " << L1_SIZE << ", Ёмкость L2 = " << L2_SIZE << "\n\n";

    md << "| Алгоритм / Комбинация |";

    md << "Тест 1 (радномные числа) |";
    md << "Тест 2 (возрастание) |";
    md << "Тест 3 (убывание) |";
    md << "Тест 4 (почти отсортированные) |";
    md << "Тест 5 (много повторов)|";
    /*
    for (size_t i = 0; i < test_data.size(); ++i) {
        md << " Тест " << i + 1 << " (N=" << test_data[i].size() << ") |";
    }*/
    md << "\n| :--- |";
    for (size_t i = 0; i < test_data.size(); ++i) md << " :---: |";
    md << "\n";

    md << "| **Одноуровневые кэши (L1)** |";
    for (size_t i = 0; i < test_data.size(); ++i) md << " |";
    md << "\n";

    for (const auto& res : results_l1) {
        md << "| " << res.name << " |";
        for (size_t h : res.hits) md << " " << h << " |";
        md << "\n";
    }

    md << "| *Идеал L1 (Белади)* |";
    for (size_t h : ideal_l1_hits) md << " **" << h << "** |";
    md << "\n";

    md << "| **Двухуровневые кэши (L1+L2)** |";
    for (size_t i = 0; i < test_data.size(); ++i) md << " |";
    md << "\n";

    for (const auto& res : results_l2) {
        md << "| " << res.name << " |";
        for (size_t h : res.hits) md << " " << h << " |";
        md << "\n";
    }

    md << "| *Идеал L1+L2 (Белади)* |";
    for (size_t h : ideal_l2_hits) md << " **" << h << "** |";
    md << "\n";

    std::cout << "[TEST] Завершено! Сохранено в 'cache_results.md'.\n";
}

int cache_start(int argc, char* argv[]) {
    try {
        if (argc > 2 && std::string(argv[1]) == "-test") {
            run_unit_tests(argv[2]);
            return 0;
        }

        std::string filename = (argc > 1) ? argv[1] : "txt/input.txt";
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Ошибка: Не удалось открыть файл '" << filename << "'\n";
            return 1;
        }

        size_t cache_count;
        if (!(file >> cache_count)) {
            std::cerr << "Ошибка: не указано количество кешей\n";
            return 1;
        }

        multi_cache_t<int, int> cache;

        for (size_t i = 0; i < cache_count; ++i) {
            std::string cache_type;
            int capacity;

            if (!(file >> cache_type >> capacity) || capacity <= 0) return 1;

            if (cache_type == "lru") cache.add_cache<lru_cache_t<int, int>>(capacity);
            else if (cache_type == "2q") cache.add_cache<two_q_cache_t<int, int>>(capacity);
            else if (cache_type == "lfu") cache.add_cache<lfu_cache_t<int, int>>(capacity);
            else if (cache_type == "lirs") cache.add_cache<lirs_cache_t<int, int>>(capacity);
            else if (cache_type == "arc") cache.add_cache<arc_cache_t<int, int>>(capacity);
            else return 1;
        }

        run_simulation(cache, file);
        cache.print_stats();
        cache.print_cache(cache.size());

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << "\n";
        return 1;
    }
}
