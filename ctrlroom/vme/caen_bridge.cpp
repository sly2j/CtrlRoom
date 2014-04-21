#include "caen_bridge.hpp"

#include <map>

using namespace ctrlroom::vme;

namespace {
    // Helper functions to determine the bridge board type

    // See comments for CVBoardTypes in CAENVMEtypes.h for info
    const std::map<std::string, CVBoardTypes> BOARD_TYPES {
        {"CAEN_VX1718", cvV1718},
        {"CAEN_VX2718", cvV2718}};
}

caen_bridge::caen_bridge(
        const std::string& identifier,
        const ptree& settings)
    : caen_bridge::base_type{identifier, settings} 
    , model_ {conf_.get(board::MODEL_KEY, BOARD_TYPES)}
    , irq_mask_ {calc_irq_mask()} {
        init();
    }

caen_bridge::~caen_bridge() {
    end();
}

void caen_bridge::wait_for_irq() const {
    CVErrorCodes err = CAENVME_IRQEnable(handle_, irq_mask_);
    HANDLE_CAEN_ERROR(err, "Failed to enable IRQ on bridge");
    err = CAENVME_IRQWait(handle_, irq_mask_, timeout_);
    HANDLE_CAEN_ERROR(err, "Problem waiting for IRQ");
}

void caen_bridge::init() {
    CVErrorCodes err = CAENVME_Init(
            model_, 
            link_index_, 
            board_index_, 
            &handle_);
    HANDLE_CAEN_ERROR(err, "Failed to initialize bridge");
}

void caen_bridge::end() {
    CVErrorCodes err = CAENVME_End(handle_);
    HANDLE_CAEN_ERROR(err, "Failed to close bridge");
}

uint32_t caen_bridge::calc_irq_mask() const {
    uint32_t mask {0};
    for (const auto& level : irq_) {
        mask |= static_cast<uint32_t>(level);
    }
    return mask;
}

error caen_bridge::decode_error(
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
