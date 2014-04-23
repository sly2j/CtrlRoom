#ifndef CTRLROOM_UTIL_MIXIN_LOADED
#define CTRLROOM_UTIL_MIXIN_LOADED

// set of CRTP mixins-from-above to flesh out a class with
// additional functionality

namespace ctrlroom {
    // add !=, <=, > and >= for to a class that defines == and <
    template <class T>
        class comparison_mixin {
            public:
                bool operator!= (const T& rhs) const {
                    return !(derived() == rhs);
                }
                bool operator<= (const T& rhs) const {
                    return (derived() == rhs || derived() < rhs);
                }
                bool operator> (const T& rhs) const {
                    return !(derived() <= rhs);
                }
                bool operator>= (const T& rhs) const {
                    return !(derived() < rhs);
                }

            private:
                const T& derived() const {
                    return static_cast<const T&>(*this);
                }
        };
    // add postfix increment/decrement operator to class that derives prefix
    // operators ++/--
    template <class T>
        class postfix_mixin {
            public:
                T operator++ (int) {
                    T tmp {derived()};
                    ++derived();
                    return tmp;
                }
                T operator-- (int) {
                    T tmp {derived()};
                    --derived();
                    return tmp;
                }
            private:
                T& derived() {
                    return static_cast<T&>(*this);
                }
        };
    // add operator ++, --, -=, + and - to classes defining +=
    // and a difference_type
    template <class T>
        class add_subtract_mixin {
            public:
                using difference_type = ptrdiff_t;
                T& operator++ () {
                    return derived() += 1;
                }
                T& operator-- () {
                    return derived() += -1;
                }
                T& operator-= (const difference_type n) {
                    return derived() += -n;
                }
                T operator+ (const difference_type n) const {
                    T tmp {derived()};
                    tmp += n;
                    return tmp;
                }
                T operator- (const difference_type n) const {
                    return operator+(-n);
                }
            private:
                T& derived() {
                    return static_cast<T&>(*this);
                }
                const T& derived() const {
                    return static_cast<const T&>(*this);
                }
        };
}



#endif
