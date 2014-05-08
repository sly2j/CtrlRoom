#include "io.hpp"

namespace ctrlroom {
//////////////////////////////////////////////////////////////////////////////////////////
// filename
//////////////////////////////////////////////////////////////////////////////////////////
std::string make_filename(const std::string &dir, const std::string &base,
                          const std::string &extra) {
  std::string ret{ dir };
  if (dir.back() != '/') {
    ret += "/";
  }
  ret += base;
  if (!extra.empty()) {
    ret += "." + extra;
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////////////////////

io_error::io_error(const std::string &msg, const std::string &type)
    : exception{ msg, type } {}

io_read_error::io_read_error(const std::string &msg, const std::string &type)
    : io_error{ msg, type } {}

io_write_error::io_write_error(const std::string &msg, const std::string &type)
    : io_error{ msg, type } {}
}
