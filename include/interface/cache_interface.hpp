#ifndef CACHE_CACHE_INTERFACE_HPP
#define CACHE_CACHE_INTERFACE_HPP

#include <optional>
#include <utility>
#include <iostream>

template <typename KeyT, typename ValueT>
class cache_interface {
public:
    virtual ~cache_interface() = default;

    virtual std::optional<ValueT> lookup(const KeyT& key) = 0;
    virtual std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) = 0;
    virtual void erase(const KeyT& key) = 0;
    virtual void print_cache(std::ostream& os = std::cout) const = 0;
};

#endif // CACHE_CACHE_INTERFACE_HPP
