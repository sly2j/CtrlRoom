#ifndef CTRLROOM_UTIL_STRINGIFY_LOADED
#define CTRLROOM_UTIL_STRINGIFY_LOADED

#include <ctrlroom/util/type_traits.hpp>

#include <boost/lexical_cast.hpp>

#include <string>
#include <utility>
#include <type_traits>

namespace ctrlroom {
namespace stringify_impl {
// standard accessor returns el untouched
template <class RangeElement, class = typename std::enable_if<
                                  !is_container<RangeElement>::value>::type>
RangeElement element_accessor(const RangeElement &el);
// container accessor returns stringified version of container
template <class RangeElement, class = typename std::enable_if<
                                  is_container<RangeElement>::value>::type,
          class = void>
RangeElement element_accessor(const RangeElement &el);
// pair element accessor returns a "<first>: <second>" string
template <class U, class V>
std::string element_accessor(const std::pair<U, V> &p);
}

// return stringified version of a range, with the elements
// separated by <delimiter>. The elements are accessed by <acc>
// prior to string conversion.
// Default accessor just returns the string, with specializations for
// STL containers and std::pair, allowing for nesting.
template <class Range,
          class ElementAccessor = decltype(
              stringify_impl::element_accessor<typename Range::value_type>)>
std::string
stringify(const Range &r, const std::string &delimiter = ", ",
          ElementAccessor acc =
              stringify_impl::element_accessor<typename Range::value_type>);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
namespace stringify_impl {
template <class RangeElement, class>
RangeElement element_accessor(const RangeElement &el) {
  return el;
}
template <class RangeElement, class, class>
RangeElement element_accessor(const RangeElement &el) {
  return "(" + stringify(el) + ")";
}
template <class U, class V>
std::string element_accessor(const std::pair<U, V> &p) {
  return (boost::lexical_cast<std::string>(element_accessor(p.first)) + ": " +
          boost::lexical_cast<std::string>(element_accessor(p.second)));
}
}

template <class Range, class ElementAccessor>
std::string stringify(const Range &r, const std::string &delimiter,
                      ElementAccessor acc) {
  std::string str{};
  for (auto &el : r) {
    if (!str.empty()) {
      str += delimiter;
    }
    str += boost::lexical_cast<std::string>(acc(el));
  }
  return str;
}
}

#endif
