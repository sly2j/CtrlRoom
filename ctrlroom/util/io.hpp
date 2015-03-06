#ifndef CTRLROOM_UTIL_IO_LOADED
#define CTRLROOM_UTIL_IO_LOADED

#include <ctrlroom/util/exception.hpp>
#include <ctrlroom/util/configuration.hpp>

#include <string>

namespace ctrlroom {

////////////////////////////////////////////////////////////////////////////////
// Exceptions
////////////////////////////////////////////////////////////////////////////////
class io_error;
class io_read_error;
class io_write_error;

////////////////////////////////////////////////////////////////////////////////
// make_filename
////////////////////////////////////////////////////////////////////////////////
std::string make_filename(const std::string& dir, const std::string& base,
                          const std::string& extra = "");

////////////////////////////////////////////////////////////////////////////////
// directory proxies
////////////////////////////////////////////////////////////////////////////////
// output directory proxy, creating the directory if necessary
// throws an exception when the directory already exists if force_new is set to
// true
struct output_directory {
  output_directory(const configuration& conf, const std::string& key,
                   bool force_new = true);
  output_directory(const std::string& path_name, bool force_new = true);
  const std::string path;
};
// input directory proxy, throws exception if the directory does not exist
struct input_directory {
  input_directory(const configuration& conf, const std::string& key);
  input_directory(const std::string& path_name);
  const std::string path;
};

} // namespace ctrlroom

////////////////////////////////////////////////////////////////////////////////
// Definition: exceptions
////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
// exceptions
class io_error : public ctrlroom::exception {
public:
  io_error(const std::string& msg, const std::string& type = "io_error");
};
class io_read_error : public io_error {
public:
  io_read_error(const std::string& msg = "Failed to read file",
                const std::string& type = "io_read_error");
};
class io_write_error : public io_error {
public:
  io_write_error(const std::string& msg = "Failed to write to file",
                 const std::string& type = "io_write_error");
};
}

// include all subheaders only if io.hpp is included directly
#ifndef CTRLROOM_UTIL_IO_INTERNAL
#include <ctrlroom/util/io/array.hpp>
#endif

#endif
