#ifndef CTRLROOM_UTIL_TYPE_TRAITS_LOADED
#define CTRLROOM_UTIL_TYPE_TRAITS_LOADED

#include <type_traits>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ctrlroom {
// is_container evaluates to std::true_type for ITERABLE STL containers
// and basic arrays
template <class T> struct is_container : std::false_type {};
// basic array
template <class T, size_t N> struct is_container<T[N]> : std::true_type {};
// std::array
template <class T, size_t N>
struct is_container<std::array<T, N>> : std::true_type {};
// std::dequeue
template <class T, class Alloc>
struct is_container<std::deque<T, Alloc>> : std::true_type {};
// std::forward_list
template <class T, class Alloc>
struct is_container<std::forward_list<T, Alloc>> : std::true_type {};
// std::list
template <class T, class Alloc>
struct is_container<std::list<T, Alloc>> : std::true_type {};
// std::map
template <class Key, class T, class Compare, class Alloc>
struct is_container<std::map<Key, T, Compare, Alloc>> : std::true_type {};
// std::multimap
template <class Key, class T, class Compare, class Alloc>
struct is_container<std::multimap<Key, T, Compare, Alloc>> : std::true_type {};
// std::set
template <class Key, class Compare, class Alloc>
struct is_container<std::set<Key, Compare, Alloc>> : std::true_type {};
// std::multiset
template <class Key, class Compare, class Alloc>
struct is_container<std::multiset<Key, Compare, Alloc>> : std::true_type {};
// std::unordered_map
template <class Key, class T, class Hash, class Pred, class Alloc>
struct is_container<std::unordered_map<Key, T, Hash, Pred, Alloc>>
    : std::true_type {};
// std::unordered_multimap
template <class Key, class T, class Hash, class Pred, class Alloc>
struct is_container<std::unordered_multimap<Key, T, Hash, Pred, Alloc>>
    : std::true_type {};
// std::unordered_set
template <class Key, class Hash, class Pred, class Alloc>
struct is_container<std::unordered_set<Key, Hash, Pred, Alloc>>
    : std::true_type {};
// std::unordered_multiset
template <class Key, class Hash, class Pred, class Alloc>
struct is_container<std::unordered_multiset<Key, Hash, Pred, Alloc>>
    : std::true_type {};
// std::vector
template <class T, class Alloc>
struct is_container<std::vector<T, Alloc>> : std::true_type {};

// is_map returns true for maps, fals for everything else
template <class T> struct is_map : std::false_type {};
template <class T, class Alloc>
struct is_map<std::map<T, Alloc>> : std::true_type {};
}

#endif
