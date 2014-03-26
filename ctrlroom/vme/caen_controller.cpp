#include "caen_controller.hpp"

#include <map>

using namespace ctrlroom::vme;

namespace {
    // Helper functions to determine the controller board type

    // See comments for CVBoardTypes in CAENVMEtypes.h for info
    const std::map<std::string, CVBoardTypes> BOARD_TYPES {
        {"V1718", cvV1718},
        {"V2718", cvV2718},
        {"A2818", cvV2818},
        {"A2719", cvV2719},
        {"A3818", cvV3818}};
}

caen_controller::caen_controller(
        const std::string& identifier,
        const ptree& settings)
    : caen_controller::base_type{identifier, settings} {

    }

caen_controller::setup() {
    model_ = conf.get(configuration::MODEL_KEY, BOARD_TYPES);


}
