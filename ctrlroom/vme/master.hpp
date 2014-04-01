#ifndef CTRLROOM_VME_MASTER_LOADED
#define CTRLROOM_VME_MASTER_LOADED

#include <ctrlroom/board.hpp>
#include <string>

namespace ctrlroom {
    namespace vme {

        class error;
        class bus_error;
        class comm_error;
        class invalid_parameter;
        class timeout_error;

        // data transfer cycle protocols,
        // for use with the vme::master::cycle* calls
        // (cycle, read_cycle, write_cycle)
        //      cycle:
        //          - RMW
        //          - ADO
        //          - ADOH
        //      read_cycle/write_cycle:
        //          - SINGLE
        //          - BLT
        //          - MBLT
        //          - MD32
        //          - P2EVME
        enum class protocol {
            SINGLE,         // read/write cycle                     -> read_cycle/write_cycle
            RMW,            // read-modify-write cycle              -> cycle
            BLT,            // Block-transfer cycle                 -> read_cycle/write_cycle
            MBLT,           // Multiplexed block transfer cycle     -> read_cycle/write_cycle
            MD32,           // Multiplexed data-32 cycle            -> read_cycle/write_cycle
            ADO,            // Address-only cycle                   -> cycle
            ADOH,           // Address-only with handshake cycle    -> cycle
            P2EVME          // Two-edge VMEbus (2eVME) cycle        -> read_cycle/write_cycle
        };

        template <class MasterImpl>
            class master : public board {
                public:
                    constexpr static const char* LINK_INDEX_KEY {"linkIndex"};
                    constexpr static const char* BOARD_INDEX_KEY {"boardIndex"};

                    typedef MasterImpl board_type;

                    master(const std::string& identifier,
                           const ptree& settings);


                    void save_settings(ptree& settings) const;

                    const std::string& name() const {
                        return name_;
                    }

                    template <protocol p> void cycle();
                    template <protocol p> void read_cycle();
                    template <protocol p> void write_cycle();

                    vme::error error(
                            const std::string& msg) const;
                    vme::bus_error bus_error(
                            const std::string& msg) const;
                    vme::comm_error comm_error(
                            const std::string& msg) const;
                    vme::invalid_parameter invalid_parameter(
                            const std::string& msg) const;
                    vme::timeout_error timeout_error(
                            const std::string& msg) const;

                protected:

                    configuration conf_;
                    const std::string name_;

                    const short optical_link_index_;
                    const short board_index_;

                private:
                    board_type& impl() {
                        return static_cast<board_type&>(*this);
                    }
                    const board_type& impl() const {
                        return static_cast<const board_type&>(*this);
                    }

                    template <class Error>
                        Error error_helper(const std::string& msg) const {
                            return {name_, optical_link_index_, board_index_, msg};
                        }
            };
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {

        // exceptions
        class error
            : public ctrlroom::exception {
                public:
                    error(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg,
                            const std::string& type="error");
            };
        class bus_error
            : public error {
                public:
                    bus_error(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg);
            };
        class comm_error
            : public error {
                public:
                    comm_error(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg);
            };
        class invalid_parameter
            : public error {
                public:
                    invalid_parameter(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg);
            };
        class timeout_error
            : public error {
                public:
                    timeout_error(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg);
            };

        // controller<> implementation
        template <class MasterImpl>
            master<MasterImpl>::master(
                    const std::string& identifier,
                    const ptree& settings)
                : conf_ {identifier, settings}
                , name_ {identifier}
                , optical_link_index_ {conf_.get<short>(OPTICAL_LINK_KEY)}
                , board_index_ {conf_.get<short>(BOARD_INDEX_KEY)} {}

        template <class MasterImpl>
            void master<MasterImpl>::save_settings(ptree& settings) const {
                conf_.save(settings);
            }

        template <class MasterImpl>
            vme::error master<MasterImpl>::error(
                    const std::string& msg) const {
                return error_helper<vme::error>(msg);
            }
        template <class MasterImpl>
            vme::bus_error master<MasterImpl>::bus_error(
                    const std::string& msg) const {
                return error_helper<vme::bus_error>(msg);
            }
        template <class MasterImpl>
            vme::comm_error master<MasterImpl>::comm_error(
                    const std::string& msg) const {
                return error_helper<vme::comm_error>(msg);
            }
        template <class MasterImpl>
            vme::invalid_parameter master<MasterImpl>::invalid_parameter(
                    const std::string& msg) const {
                return error_helper<vme::invalid_parameter>(msg);
            }
        template <class MasterImpl>
            vme::timeout_error master<MasterImpl>::timeout_error(
                    const std::string& msg) const {
                return error_helper<vme::timeout_error>(msg);
            }
    }
}

#endif
