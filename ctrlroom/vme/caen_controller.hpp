#ifndef CTRLROOM_VME_CAEN_CONTROLLER_LOADED
#define CTRLROOM_VME_CAEN_CONTROLLER_LOADED

#include <ctrlroom/controller.hpp>
#include <CAENVMElib.h>
#include <cstddef>
#include <string>

namespace ctrlroom {
    namespace vme {
        class caen_controller
            : controller<caen_controller>
        {
            public:
                typedef controller<caen_controller> base_type;

                controller(const std::string& name,
                        const CVBoardTypes model)
                    : base_type(name)
                    , model_ {model}


            private:
                int32_t handle_;
                CVBoardTypes model_;
        };
    }
}
