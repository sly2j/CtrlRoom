#include "master.hpp"

namespace ctrlroom {
namespace vme {
const translation_map<irq_level> IRQ_TRANSLATOR{ { "IRQ1", irq_level::IRQ1 },
                                                 { "IRQ2", irq_level::IRQ2 },
                                                 { "IRQ3", irq_level::IRQ3 },
                                                 { "IRQ4", irq_level::IRQ4 },
                                                 { "IRQ5", irq_level::IRQ5 },
                                                 { "IRQ6", irq_level::IRQ6 },
                                                 { "IRQ7", irq_level::IRQ7 } };
}
};

using namespace ctrlroom::vme;

////////////////////////////////////////////////////////////////////////////////
// exceptions
////////////////////////////////////////////////////////////////////////////////
error::error(const std::string &board_name, const short link_index,
             const short board_index, const std::string &msg,
             const std::string &type)
    : ctrlroom::exception(msg + " (for '" + board_name + "', link: " +
                              std::to_string(link_index) + ", board: " +
                              std::to_string(board_index) + ")",
                          type) {}
bus_error::bus_error(const std::string &board_name, const short link_index,
                     const short board_index, const std::string &msg)
    : vme::error{ board_name, link_index, board_index, msg, "vme::bus_error" } {
}
comm_error::comm_error(const std::string &board_name, const short link_index,
                       const short board_index, const std::string &msg)
    : vme::error{
        board_name, link_index, board_index, msg, "vme::comm_error"
      } {}
invalid_parameter::invalid_parameter(const std::string &board_name,
                                     const short link_index,
                                     const short board_index,
                                     const std::string &msg)
    : vme::error{ board_name, link_index,              board_index,
                  msg,        "vme::invalid_parameter" } {}
timeout_error::timeout_error(const std::string &board_name,
                             const short link_index, const short board_index,
                             const std::string &msg)
    : vme::error{
        board_name, link_index, board_index, msg, "vme::timeout_error"
      } {}
