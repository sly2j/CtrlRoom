#ifndef CTRLROOM_VME_CONTROLLER_LOAEDED
#define CTRLROOM_VME_CONTROLLER_LOAEDED

#include <ctrlroom/util/configuration.hpp>
#include <string>

namespace ctrlroom {
    namespace vme {

        class controller_error;
        class controller_bus_error;
        class controller_comm_error;
        class controller_invalid_parameter;
        class controller_timeout_error;

        template <class CtrlImpl>
            class controller {
                public:
                    constexpr static const char* OPTICAL_LINK_KEY {"opticalLinkIndex"};
                    constexpr static const char* BOARD_INDEX_KEY {"boardIndex"};

                    typedef CtrlImpl controller_type;

                    controller(const std::string& identifier,
                               const ptree& settings);


                    void save_settings(ptree& settings) const;



                    const std::string& name() const {
                        return name_;
                    }

                    controller_error error(
                            const std::string& msg) const;
                    controller_bus_error bus_error(
                            const std::string& msg) const;
                    controller_comm_error comm_error(
                            const std::string& msg) const;
                    controller_invalid_parameter invalid_parameter(
                            const std::string& msg) const;
                    controller_timeout_error timeout_error(
                            const std::string& msg) const;

                protected:

                    configuration conf_;
                    const std::string name_;

                    const short optical_link_index_;
                    const short board_index_;

                private:
                    controller_type& impl() {
                        return static_cast<controller_type&>(*this);
                    }
                    const controller_type& impl() const {
                        return static_cast<const controller_type&>(*this);
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
        class controller_error
            : public ctrlroom::exception {
                public:
                    controller_error(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg,
                            const std::string& type="controller_error");
            };
        class controller_bus_error
            : public controller_error {
                public:
                    controller_bus_error(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg);
            };
        class controller_comm_error
            : public controller_error {
                public:
                    controller_comm_error(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg);
            };
        class controller_invalid_parameter
            : public controller_error {
                public:
                    controller_invalid_parameter(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg);
            };
        class controller_timeout_error
            : public controller_error {
                public:
                    controller_timeout_error(
                            const std::string& board_name,
                            const short link_index,
                            const short board_index,
                            const std::string& msg);
            };

        // controller<> implementation
        template <class CtrlImpl>
            controller<CtrlImpl>::controller(
                    const std::string& identifier,
                    const ptree& settings)
                : conf_ {identifier, settings}
                , name_ {identifier}
                , optical_link_index_ {conf_.get<short>(OPTICAL_LINK_KEY)}
                , board_index_ {conf_.get<short>(BOARD_INDEX_KEY)} {}

        template <class CtrlImpl>
            void controller<CtrlImpl>::save_settings(ptree& settings) const {
                conf_.save(settings);
            }

        template <class CtrlImpl>
            controller_error controller<CtrlImpl>::error(
                    const std::string& msg) const {
                return error_helper<controller_error>(msg);
            }
        template <class CtrlImpl>
            controller_bus_error controller<CtrlImpl>::bus_error(
                    const std::string& msg) const {
                return error_helper<controller_bus_error>(msg);
            }
        template <class CtrlImpl>
            controller_comm_error controller<CtrlImpl>::comm_error(
                    const std::string& msg) const {
                return error_helper<controller_comm_error>(msg);
            }
        template <class CtrlImpl>
            controller_invalid_parameter controller<CtrlImpl>::invalid_parameter(
                    const std::string& msg) const {
                return error_helper<controller_invalid_parameter>(msg);
            }
        template <class CtrlImpl>
            controller_timeout_error controller<CtrlImpl>::timeout_error(
                    const std::string& msg) const {
                return error_helper<controller_timeout_error>(msg);
            }
    }
}

#endif
