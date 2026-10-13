#ifndef CACHE_CACHE_INTERFACE_HPP
#define CACHE_CACHE_INTERFACE_HPP

#include <optional>
#include <utility>

template <typename KeyT, typename ValueT>
class cache_interface {
public:
    virtual ~cache_interface() = default;

    virtual bool lookup(const KeyT& key, ValueT& value) = 0;
    virtual std::optional<std::pair<KeyT, ValueT>> insert(const KeyT& key, const ValueT& value) = 0;
    virtual void erase(const KeyT& key) = 0;
    virtual void print_cache() const = 0;
};

#endif
