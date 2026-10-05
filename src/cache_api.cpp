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

int cache_start(int argc, char* argv[]) {
    try {
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
