#ifndef CTRLROOM_VME_CAEN_CONTROLLER_LOADED
#define CTRLROOM_VME_CAEN_CONTROLLER_LOADED

#include <ctrlroom/util/exception.hpp>
#include <ctrlroom/util/configuration.hpp>
#include <ctrlroom/vme/controller.hpp>

#include <CAENVMElib.h>
#include <cstddef>
#include <string>


// Throw a controller_error if a problem was encountered.
// Uses macro instead of inline function to avoid unnecessary 
// instantiation of msg in tight loops
#define HANDLE_CAEN_ERROR(err, msg) \
    if ((err)) {throw decode_error((err), (msg));}

namespace ctrlroom {
    namespace vme {

        class caen_controller
            : vme::controller<caen_controller> {
                public:
                    typedef vme::controller<caen_controller> base_type;

                    caen_controller(
                            const std::string& identifier,
                            const ptree& settings);

                    ~caen_controller();


                protected:
                    const CVBoardTypes model_;
                    

                private:
                    void setup();
                    void init();
                    void end();

                    controller_error decode_error(
                            const CVErrorCodes err,
                            const std::string& msg) const;

                    int32_t handle_;
        };
    }
}

#endif
