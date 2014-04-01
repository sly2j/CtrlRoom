#include "controller.hpp"

using namespace ctrlroom::vme;

//////////////////////////////////////////////////////////////////////////////////////////
// exceptions
//////////////////////////////////////////////////////////////////////////////////////////
controller_error::controller_error(
        const std::string& board_name,
        const short link_index,
        const short board_index,
        const std::string& msg,
        const std::string& type)
    : ctrlroom::exception(
            msg + " (for " + board_name
                + ", link: " + std::to_string(link_index)
                + ", board: " + std::to_string(board_index) + ")",
            type) {}
controller_bus_error::controller_bus_error(
        const std::string& board_name,
        const short link_index,
        const short board_index,
        const std::string& msg)
    : controller_error{
        board_name, link_index, board_index,
        msg, "controller_bus_error"} {}
controller_comm_error::controller_comm_error(
        const std::string& board_name,
        const short link_index,
        const short board_index,
        const std::string& msg)
    : controller_error{
        board_name, link_index, board_index,
        msg, "controller_comm_error"} {}
controller_invalid_parameter::controller_invalid_parameter(
        const std::string& board_name,
        const short link_index,
        const short board_index,
        const std::string& msg)
    : controller_error{
        board_name, link_index, board_index,
        msg, "controller_invalid_parameter"} {}
controller_timeout_error::controller_timeout_error(
        const std::string& board_name,
        const short link_index,
        const short board_index,
        const std::string& msg)
    : controller_error{
        board_name, link_index, board_index,
        msg, "controller_timeout_error"} {}
