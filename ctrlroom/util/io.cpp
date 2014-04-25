#include "io.hpp"

using namespace ctrlroom;

//////////////////////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////////////////////

io_error::io_error(
        const std::string& msg,
        const std::string& type)
    : exception{msg, type} {} 

io_read_error::io_read_error(
        const std::string& msg,
        const std::string& type)
    : io_error{msg, type} {}

io_write_error::io_write_error(
        const std::string& msg,
        const std::string& type)
    : io_error{msg, type} {}
