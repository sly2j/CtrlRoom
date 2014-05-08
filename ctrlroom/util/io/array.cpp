#include "array.hpp"

using namespace ctrlroom;

//////////////////////////////////////////////////////////////////////////////////////////
// Implementation: exceptions
//////////////////////////////////////////////////////////////////////////////////////////

io_array_read_error::io_array_read_error(const std::string &msg,
                                         const std::string &type)
    : io_read_error{ msg, type } {}
io_array_write_error::io_array_write_error(const std::string &msg,
                                           const std::string &type)
    : io_write_error{ msg, type } {}
io_array_length_error::io_array_length_error(const size_t length,
                                             const std::string &type)
    : io_array_read_error{ "Failed to read array of length " +
                               std::to_string(length) + " from file",
                           type } {}
