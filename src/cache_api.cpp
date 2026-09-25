#include "cache.hpp"
#include "cache_api.hpp"
#include <fstream>


int slow_get_page(int key) {
    return (key);
}

template <typename KeyT = int, typename ValueT = int>
void run_simulation(multi_cache_t<KeyT, ValueT>& cache, std::istream& is) {
    KeyT page_key;
    ValueT value;
    
    while (is >> page_key) {
        value = slow_get_page(page_key);
        cache.request(page_key, value);
    }

    if (!is.eof() && is.fail()) {
        std::cerr << "Ошибка: встречен некорректный символ во входном файле.\n";
        exit(1);
    }
}

int cache_start(int argc, char* argv[]) {
    try {
        std::string filename = (argc > 1) ? argv[1] : "txt/input.txt";

        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Ошибка: Не удалось открыть файл " << filename << "'\n";
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

            if (!(file >> cache_type >> capacity)) {
                std::cerr << "Ошибка: некорректная конфигурация кеша\n";
                return 1;
            }

            if (capacity <= 0) {
                std::cerr << "Ошибка: capacity должна быть > 0\n";
                return 1;
            }

            if (cache_type == "lru") {
                cache.add_cache<lru_cache_t<int, int>>(capacity);
            }
            else if (cache_type == "2q") {
                cache.add_cache<two_q_cache_t<int, int>>(capacity);
            }
            else if (cache_type == "lfu") {
                cache.add_cache<lfu_cache_t<int, int>>(capacity);
            }
            else if (cache_type == "lirs") {
                cache.add_cache<lirs_cache_t<int, int>>(capacity);
            }
            else if (cache_type == "arc") {
                cache.add_cache<arc_cache_t<int, int>>(capacity);
            }
            else {
                std::cerr << "Ошибка: неизвестный тип кеша '"
                          << cache_type << "'\n";
                return 1;
            }
        }

        run_simulation(cache, file);
        cache.print_stats(); 

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << "\n";
        return 1;
    }
}
