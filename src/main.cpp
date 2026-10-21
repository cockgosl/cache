#include "interface/cache_api.hpp"

#include "cache_algo/lru_cache.hpp"
#include "cache_algo/lfu_cache.hpp"
#include "cache_algo/two_q_cache.hpp"
#include "cache_algo/arc_cache.hpp"
#include "cache_algo/lirs_cache.hpp"

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

namespace{
void run_simulation(multi_cache_t<int, int>& cache, std::istream& is) {
    int page_key = 0;
    int value = 0;
    while (is >> page_key) {
        cache.request_inclusive(page_key, value);
    }
    if (!is.eof() && is.fail()) {
        throw std::runtime_error("Error: an invalid character was encountered in the input file.");
    }
}
}

int main(int argc, char* argv[]) try {
    std::string filename = (argc > 1) ? argv[1] : "txt/input.txt";
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ERROR: can't open file'" << filename << "'\n";
        return 1;
    }

    size_t cache_count;
    if (!(file >> cache_count)) {
        std::cerr << "ERROR: didn't mentioned cache amount\n";
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
    std::cerr << "Exception: " << e.what() << '\n';
    return 1;
}
catch(...){
    return 1;
}
