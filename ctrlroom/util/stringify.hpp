#ifndef CTRLROOM_UTIL_STRINGIFY_LOADED
#define CTRLROOM_UTIL_STRINGIFY_LOADED

#include <string>
#include <utility>
#include <boost/lexical_cast.hpp>

namespace ctrlroom {
    namespace stringify_impl {
        template <class U, class V>
            std::string element_accessor(const std::pair<U, V>& p);
        template <class RangeElement>
            RangeElement element_accessor(const RangeElement& el);
    }

    template <class Range, class ElementAccessor>
        std::string stringify(
                const Range& r,
                const std::string& delimiter=", ",
                ElementAccessor acc=stringify_impl::element_accessor<
                    typename Range::value_type>);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom
{
    namespace stringify_impl {
        template <class U, class V>
            std::string element_accessor(const std::pair<U, V>& p) {
                return (boost::lexical_cast<std::string>(p.first)
                            + ": "
                            + boost::lexical_cast<std::string>(p.second));
            }
        template <class RangeElement>
            RangeElement element_accessor(const RangeElement& el) {
                return el;
            }
        }

    template <class Range, class ElementAccessor>
        std::string stringify(
                const Range& r,
                const std::string& delimiter,
                ElementAccessor acc) {
            std::string str {};
            for (auto& el : r) {
                if (!str.empty()) {
                    str += delimiter;
                }
                str += boost::lexical_cast<std::string>(acc(el));
            }
            return str;
        }
}

#endif
