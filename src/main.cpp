#include "cache.hpp"
#include <exception>
#include <stdexcept>

LRUCache::LRUCache(size_t capacity) 
    :capacity{capacity} {
    if (capacity == 0) {
        throw std::invalid_argument("Cache capacity must be greater than 0\n");
    }
}

bool LRUCache::cache_renew(int page) {
    bool sign = 0;
    for (auto it = cache.begin(); it != cache.end(); ++it) {
        if(*it == page) {
            cache.splice(cache.begin(), cache, it);
            sign = 1;
            break;
        }
    }
    if (sign == 0) {
        if (!(cache.size() < capacity)) {
            cache.pop_back();
        }
        cache.push_front(page);
    }
    return sign;
}

void LRUCache::print_cache() {
    for (int x : cache) {
        std::cout << x;
    }
    std::cout << "\n";
}

size_t cache_func(size_t capacity, std::istream& is) {

    if (!is) {
        throw std::runtime_error{"Invalid input stream\n"};
    }    

    size_t hits = 0;
    int page = 0;
    LRUCache cache(capacity);
    while(is >> page) {
        hits += cache.cache_renew(page);
        cache.print_cache();
    }

    if(!is.eof()) {
        throw std::runtime_error{"incorrect input\n"};
    }
    return hits;
}


int main() {
    try {
        std::ifstream file("txt/input.txt");
        std::cout << cache_func(5, file);
    }
    catch(const std::exception& e) {
        std::cerr << "error:" << e.what() << "\n";
    }
    return 0;
}
