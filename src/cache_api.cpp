#include <cache.hpp>
#include <fstream>

int slow_get_page(int key) {
    return key;
}

template <typename CacheT>
size_t run_simulation(size_t capacity, std::istream& is) {
    CacheT cache(capacity);
    size_t hits = 0;
    int page_key = 0;

    // Считываем числа из файла
    while (is >> page_key) {
        if (cache.lookup_update(page_key, slow_get_page)) {
            hits++;
        }
    }

    // Проверяем, что файл прочитан до конца без ошибок формата
    if (!is.eof() && is.fail()) {
        std::cerr << "Ошибка: встречен некорректный символ во входном файле.\n";
        exit(1);
    }

    return hits;
}
int cache_start(int argc, char* argv[]) {
     try {
        // Указываем путь к файлу (по умолчанию "input.txt" или из аргументов командной строки)
        std::string filename = (argc > 1) ? argv[1] : "txt/input.txt";

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Ошибка: Не удалось открыть файл '" << filename << "'\n";
            return 1;
        }

        std::string cache_type;
        size_t capacity = 0;

        // Ввод из файла: <тип_кэша> <емкость>
        if (!(file >> cache_type >> capacity)) {
            std::cerr << "Ошибка: Некорректный формат заглавных данных в файле\n";
            return 1;
        }

        size_t hits = 0;

        if (cache_type == "lru") {
            hits = run_simulation<lru_cache_t<int>>(capacity, file);
        } else if (cache_type == "2q") {
            hits = run_simulation<two_q_cache_t<int>>(capacity, file);
        } else if (cache_type == "lfu") {
            hits = run_simulation<lfu_cache_t<int>>(capacity, file);
        } else if (cache_type == "lirs") {
            hits = run_simulation<lirs_cache_t<int>>(capacity, file);
        } else if (cache_type == "arc") {
            hits = run_simulation<arc_cache_t<int>>(capacity, file);
        } else {
            // При неправильном типе алгоритма останавливаем программу
            std::cerr << "Ошибка: Неизвестный тип алгоритма кэширования '" << cache_type
                      << "'. Допустимые значения: lru, 2q, lfu, lirs, arc.\n";
            return 1;
        }

        std::cout << "Hits: " << hits << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Исключение: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

