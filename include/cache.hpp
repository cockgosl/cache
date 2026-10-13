#ifndef CACHE_HPP
#define CACHE_HPP

#include <cstddef>
#include <istream>

#include "cache_interface.hpp"

#include "LRUCache.hpp"
#include "LFUCache.hpp"
#include "2QCache.hpp"
#include "ARCCache.hpp"
#include "LIRSCache.hpp"

int cache_start(int argc, char* argv[]);
size_t run_simulation(size_t capacity, std::istream& is);

#endif // CACHE_HPP
