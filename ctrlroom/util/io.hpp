#ifndef CTRLROOM_UTIL_IO_LOADED
#define CTRLROOM_UTIL_IO_LOADED

#include <ctrlroom/util/exception.hpp>

#include <string>

namespace ctrlroom {

    class io_error;
    class io_read_error;
    class io_write_error;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Definition: exceptions
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom
{
    // exceptions
    class io_error
        : public ctrlroom::exception {
            public:
                io_error(
                        const std::string& msg,
                        const std::string& type="io_error");
        };
    class io_read_error
        : public io_error {
            public:
                io_read_error(
                        const std::string& msg="Failed to read file",
                        const std::string& type="io_read_error");
        };
    class io_write_error
        : public io_error {
            public:
                io_write_error(
                        const std::string& msg="Failed to write to file",
                        const std::string& type="io_write_error");
        };
}

// include all subheaders only if io.hpp is included directly
#ifndef CTRLROOM_UTIL_IO_INTERNAL
#   include <ctrlroom/util/io/array.hpp>
#endif

#endif
