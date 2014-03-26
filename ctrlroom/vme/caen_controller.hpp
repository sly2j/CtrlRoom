#ifndef CTRLROOM_VME_CAEN_CONTROLLER_LOADED
#define CTRLROOM_VME_CAEN_CONTROLLER_LOADED

#include <ctrlroom/util/configuration.hpp>
#include <ctrlroom/vme/controller.hpp>

#include <CAENVMElib.h>
#include <cstddef>
#include <string>

namespace ctrlroom {
    namespace vme {
        class caen_controller
            : vme::controller<caen_controller> {
                public:
                    typedef vme::controller<caen_controller> base_type;

                    caen_controller(
                            const std::string& identifier,
                            const ptree& settings);


                    

                private:
                    void setup();
                    void init();
                    void end();

                    void setup_model();

                    int32_t handle_;
                    configuration conf_;
                    CVBoardTypes model_;
        };
    }
}
