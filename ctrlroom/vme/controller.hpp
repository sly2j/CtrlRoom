#ifndef CTRLROOM_VME_CONTROLLER_LOAEDED
#define CTRLROOM_VME_CONTROLLER_LOAEDED

#include <ctrlroom/util/configuration.hpp>
#include <string>

namespace ctrlroom {
    namespace vme {

        template <class CtrlImpl>
            class controller {
                public:
                    typedef CtrlImpl controller_type;

                    controller(const std::string& identifier,
                               const ptree& settings)
                        : conf_ {identifier, settings}
                        , name_ {identifier} {}

                    void save_settings(ptree& settings) const {
                        conf_.save(settings);
                    }

                    const std::string& name() const {
                        return name_;
                    }

                protected:
                    configuration& conf() {
                        return conf_;
                    }
                    const configuration& conf() const {
                        return conf_;
                    }

                private:
                    controller_type& impl() {
                        return static_cast<controller_type&>(*this);
                    }
                    const controller_type& impl() const {
                        return static_cast<const controller_type&>(*this);
                    }

                    configuration conf_;
                    const std::string name_;

            };
    }
}

#endif
