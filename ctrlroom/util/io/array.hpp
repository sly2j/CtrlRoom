#ifndef CTRLROOM_UTIL_IO_ARRAY_LOADED
#define CTRLROOM_UTIL_IO_ARRAY_LOADED

#define CTRLROOM_UTIL_IO_INTERNAL 1

#include <ctrlroom/util/io.hpp>

#include <istream>
#include <ostream>
#include <array>

namespace ctrlroom {

    class io_array_read_error;
    class io_array_length_error;
    class io_array_write_error;

    template <class T, size_t N>
        std::array<T, N> read_array(std::istream& in);

    template <class T, size_t N>
        void write_array(
                std::ostream& out, 
                const std::array<T, N>& a,
                const size_t ncols = 1,
                const std::string& col_separator = " ");

}

//////////////////////////////////////////////////////////////////////////////////////////
// Definition: exceptions
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom
{
    // exceptions
    class io_array_read_error
        : public io_read_error {
            public:
                io_array_read_error(
                        const std::string& msg="Failed to read array from file.",
                        const std::string& type="io_array_read_error");
        };
    class io_array_write_error
        : public io_write_error {
            public:
                io_array_write_error(
                        const std::string& msg="Failed to write array to file.",
                        const std::string& type="io_array_write_error");
        };
    class io_array_length_error
        : public io_array_read_error {
            public:
                io_array_length_error(
                        const size_t length,
                        const std::string& type="io_array_length_error");
        };
}
//////////////////////////////////////////////////////////////////////////////////////////
// Implementation: read/write
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    template <class T, size_t N>
        std::array<T, N> read_array(std::istream& in) {
            if (!in) {
                throw io_array_read_error{};
            }
            std::array<T, N> a;
            for (size_t i {0}; i < N; ++i) {
                if (!in) {
                    throw io_array_length_error{N};
                }
                in >> a[i];
            }
            return a;
        }

    template <class T, size_t N>
        void write_array(
                std::ostream& out, 
                const std::array<T, N> a,
                const size_t ncols,
                const std::string& col_separator) {
            if (N % a) {
                throw io_array_write_error{};
            }
            unsigned col {0};
            for (auto val : a) {
                if (!out) {
                    throw io_array_write_error{};
                }
                if (col) {
                    out << col_separator;
                }
                out << val;
                ++col;
                if (col > ncols) {
                    col = 0;
                    out << "\n";
                }
            }
        }
}


#undef CTRLROOM_UTIL_IO_INTERNAL

#endif
