#include "caen_controller.hpp"

#include <map>

using namespace ctrlroom::vme;

namespace {
    // Helper functions to determine the controller board type

    // See comments for CVBoardTypes in CAENVMEtypes.h for info
    const std::map<std::string, CVBoardTypes> BOARD_TYPES {
        {"V1718", cvV1718},
        {"V2718", cvV2718}};
}

caen_controller::caen_controller(
        const std::string& identifier,
        const ptree& settings)
    : caen_controller::base_type{identifier, settings} 
    , model_ {conf_.get(configuration::MODEL_KEY, BOARD_TYPES)} {
        init();
    }

caen_controller::~caen_controller() {
    end();
}

        


void caen_controller::init() {
    CVErrorCodes err = CAENVME_Init(
            model_, 
            optical_link_index_, 
            board_index_, 
            &handle_);
    HANDLE_CAEN_ERROR(err, "Failed to initialize controller");
        

}

void caen_controller::end() {
    CVErrorCodes err = CAENVME_End(handle_);
    HANDLE_CAEN_ERROR(err, "Failed to close controller");
}

controller_error caen_controller::decode_error(
        const CVErrorCodes err,
        const std::string& msg) const {
    if (err == cvBusError) {
        return bus_error(msg);
    } else if (err == cvCommError) {
        return comm_error(msg);
    } else if (err == cvInvalidParam) {
        return invalid_parameter(msg);
    } else if (err == cvTimeoutError) {
        return timeout_error(msg);
    } 
    // default:
    return error(msg);
}
