#include <iostream>
#include <unordered_map>
#include <list>
#include <fstream>

void print_cache(std::list<int> cache);
size_t cache_func(size_t s, std::istream& is);

class LRUCache {
    private:
        std::list<int> cache;
        size_t capacity;
    public:
        LRUCache(size_t capacity);

        bool cache_renew(int page);
        void print_cache();
};


