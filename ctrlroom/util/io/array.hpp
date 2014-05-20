#ifndef CTRLROOM_UTIL_IO_ARRAY_LOADED
#define CTRLROOM_UTIL_IO_ARRAY_LOADED

#define CTRLROOM_UTIL_IO_INTERNAL 1

#include <ctrlroom/util/io.hpp>

#include <istream>
#include <ostream>
#include <vector>
#include <array>
#include <fstream>

namespace ctrlroom {

class io_array_read_error;
class io_array_length_error;
class io_array_write_error;

// read just one array, either from a stream
// or a filename
template <class T, size_t N>
void read_array(std::istream& in, std::array<T, N>& a);
template <class T, size_t N>
void read_array(const std::string& fname, std::array<T, N>& a);
// same for multiple arrays at once
template <class T, size_t N>
void read_array(std::istream& in, std::vector<std::array<T, N>*> arrays);
template <class T, size_t N>
void read_array(const std::string& fname,
                std::vector<std::array<T, N>*> arrays);

// write just one array, either from a stream
// or a filename
template <class T, size_t N>
void write_array(std::ostream& out, const std::array<T, N>& a,
                 const size_t ncols = 1,
                 const std::string& col_separator = " ");
template <class T, size_t N>
void write_array(const std::string& fname, const std::array<T, N>& a,
                 const size_t ncols = 1,
                 const std::string& col_separator = " ");
// same for multiple arrays at once
template <class T, size_t N>
void write_array(std::ostream& out, std::vector<std::array<T, N>> arrays,
                 const size_t ncols = 1,
                 const std::string& col_separator = " ");
template <class T, size_t N>
void write_array(const std::string& fname, std::vector<std::array<T, N>> arrays,
                 const size_t ncols = 1,
                 const std::string& col_separator = " ");
}

//////////////////////////////////////////////////////////////////////////////////////////
// Definition: exceptions
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
// exceptions
class io_array_read_error : public io_read_error {
public:
  io_array_read_error(
      const std::string& msg = "Failed to read array from file.",
      const std::string& type = "io_array_read_error");
};
class io_array_write_error : public io_write_error {
public:
  io_array_write_error(
      const std::string& msg = "Failed to write array to file.",
      const std::string& type = "io_array_write_error");
};
class io_array_length_error : public io_array_read_error {
public:
  io_array_length_error(const size_t length,
                        const std::string& type = "io_array_length_error");
};
}
//////////////////////////////////////////////////////////////////////////////////////////
// Implementation: read/write
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
template <class T, size_t N>
void read_array(std::istream& in, std::array<T, N>& a) {
  if (!in) {
    throw io_array_read_error{};
  }
  for (size_t i{0}; i < N; ++i) {
    if (!in) {
      throw io_array_length_error{N};
    }
    in >> a[i];
  }
}
template <class T, size_t N>
void read_array(const std::string& fname, std::array<T, N>& a) {
  std::ifstream f{fname};
  read_array(f, a);
}
template <class T, size_t N>
void read_array(std::istream& in, std::vector<std::array<T, N>*> arrays) {
  for (auto a : arrays) {
    if (a) {
      read_array(in, *a);
    }
  }
}
template <class T, size_t N>
void read_array(const std::string& fname,
                std::vector<std::array<T, N>*> arrays) {
  std::ifstream f{fname};
  for (auto& a : arrays) {
    if (a) {
      read_array(f, *a);
    }
  }
}

template <class T, size_t N>
void write_array(std::ostream& out, const std::array<T, N>& a,
                 const size_t ncols, const std::string& col_separator) {
  if (ncols == 0 || N % ncols) {
    throw io_array_write_error{};
  }
  unsigned col{0};
  for (auto val : a) {
    if (!out) {
      throw io_array_write_error{};
    }
    if (col) {
      out << col_separator;
    }
    out << val;
    ++col;
    if (col >= ncols) {
      col = 0;
      out << "\n";
    }
  }
}
template <class T, size_t N>
void write_array(const std::string& fname, const std::array<T, N>& a,
                 const size_t ncols, const std::string& col_separator) {
  std::ofstream f{fname};
  write_array(f, a, ncols, col_separator);
}
template <class T, size_t N>
void write_array(std::ostream& out, std::vector<std::array<T, N>> arrays,
                 const size_t ncols, const std::string& col_separator) {
  for (const auto& a : arrays) {
    write_array(out, a, ncols, col_separator);
  }
}
template <class T, size_t N>
void write_array(const std::string& fname, std::vector<std::array<T, N>> arrays,
                 const size_t ncols, const std::string& col_separator) {
  std::ofstream f{fname};
  for (const auto& a : arrays) {
    write_array(f, a, ncols, col_separator);
  }
}
}

#undef CTRLROOM_UTIL_IO_INTERNAL

#endif
