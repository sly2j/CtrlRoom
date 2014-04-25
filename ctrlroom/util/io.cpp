#include "io.hpp"

using namespace ctrlroom;

//////////////////////////////////////////////////////////////////////////////////////////
// filename
//////////////////////////////////////////////////////////////////////////////////////////
std::string make_filename(
        const std::string& dir,
        const std::string& name_root,
        const std::string& name) {
    std::string ret {dir};
    if (dir.back() != '/') {
        ret += "/";
    }
    ret += name_root + name;
    return ret;
}

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
