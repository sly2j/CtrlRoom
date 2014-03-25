#ifndef CTRLROOM_VME_CAEN_CONTROLLER_LOADED
#define CTRLROOM_VME_CAEN_CONTROLLER_LOADED

#include <ctrlroom/controller.hpp>
#include <ctrlroom/configuration.hpp>
#include <CAENVMElib.h>
#include <cstddef>
#include <string>

namespace ctrlroom {
    namespace vme {
        class caen_controller
            : controller<caen_controller> {
                public:
                    typedef controller<caen_controller> base_type;

                    caen_controller(
                            const std::string identifier,
                            const ptree& settings)
                        : base_type(identifier)
                        , conf {identifier, settings} {
                        }


                    void process_settings() {

                    }


                    

                protected:
                    void init

                private:
                    process_conf

                    int32_t handle_;
                    configuration conf_;
                    CVBoardTypes model_;
        };
    }
}
