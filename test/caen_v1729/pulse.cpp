#include <ctrlroom/vme/caen_bridge.hpp>
#include <ctrlroom/vme/caen_v1729.hpp>
#include <ctrlroom/vme/vme64.hpp>

#include <ctrlroom/util/stringify.hpp>
#include <ctrlroom/util/configuration.hpp>
#include <ctrlroom/util/exception.hpp>

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <exception>

using namespace ctrlroom;

using bridge_type = vme::caen_bridge;
using adc_type = vme::caen_v1729a<bridge_type,
                                  vme::addressing_mode::A32>;


int main() {
    
    try {
        ptree config;
        read_json("test.json", config);

        std::shared_ptr<bridge_type> master {
            new bridge_type{"master", config}
        };
        auto pedestal = adc_type::measure_pedestal("adc", config, master);
    
    } catch (exception& e) {
        LOG_ERROR(e.type(), e.what());
        return 1;
    } catch (std::exception& e ) {
        LOG_ERROR("std::exception", e.what());
        return -1;
    }
    return 0;
}
