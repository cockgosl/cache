#ifndef CACHE_HPP
#define CACHE_HPP

#include <cstddef>
#include <istream>

#include "cache_interface.hpp"

#include "cache_algo/lru_cache.hpp"
#include "cache_algo/lfu_cache.hpp"
#include "cache_algo/two_q_cache.hpp"
#include "cache_algo/arc_cache.hpp"
#include "cache_algo/lirs_cache.hpp"

int cache_start(int argc, char* argv[]);
size_t run_simulation(size_t capacity, std::istream& is);

#endif // CACHE_HPP
