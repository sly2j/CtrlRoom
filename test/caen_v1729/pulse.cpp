
#define private public
#define protected public

#include <ctrlroom/vme/caen_bridge.hpp>
#include <ctrlroom/vme/caen_v1729.hpp>
#include <ctrlroom/vme/vme64.hpp>

#include <ctrlroom/util/stringify.hpp>
#include <ctrlroom/util/configuration.hpp>
#include <ctrlroom/util/exception.hpp>
#include <ctrlroom/util/assert.hpp>
#include <ctrlroom/util/logger.hpp>

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <exception>
#include <typeinfo> //DBG

#include <TFile.h>
#include <TTree.h>

using namespace ctrlroom;

using bridge_type = vme::caen_bridge;
using adc_type = vme::caen_v1729a<bridge_type,
                                  vme::addressing_mode::A32,
                                  vme::transfer_mode::D32,      // single
                                  vme::transfer_mode::MBLT>;    // BLT


int main(int argc, char* argv[]) {

    global::logger.set_level(log_level::JUNK2);

    
    try {
        tassert(argc == 2, "One command line parameter required (output file)")
                
        ptree config;
        read_json("test.json", config);

        LOG_INFO("MAIN", "Obtaining bridge handle");
        std::shared_ptr<bridge_type> master {
            new bridge_type{"bridge", config}
        };
        LOG_INFO("MAIN", "Calibrating ADC pedestals");
        auto pedestal = adc_type::measure_pedestal("ADC", config, master);
        //adc_type::memory_type pedestal {0};
        LOG_INFO("MAIN", "Calibrating ADC verniers");
        //auto vernier = adc_type::calibrate_verniers("ADC", config, master);
        std::pair<adc_type::vernier_type, adc_type::vernier_type> vernier {
            {8588, 8680, 8649, 8588}, 
            {10293, 10429, 10385, 10299}
        };

        LOG_WARNING("MAIN", "Hookup your wires and press enter to continue...");
        std::cin.ignore();
        LOG_INFO("MAIN", "Obtaining ADC handle");
        adc_type adc {
            "ADC", 
            config, 
            master, 
            {pedestal, vernier.first, vernier.second}
        };

        adc_type::buffer_type buf;

        TFile f {argv[1], "recreate"};
        TTree t {"ramp", "ramp"};
        int event {0};
        int sample {0};
        int channel0 {0};
        t.Branch("event", &event, "event/I");
        t.Branch("sample", &sample, "sample/I");
        t.Branch("channel0", &channel0, "channel0/I");
        LOG_INFO("MAIN", "Measuring 100 pulses");
        for (int i = 0; i < 100; ++i) {
            master->wait_for_irq();
            adc.read_pulse(buf);
            ++event;
            sample = 0;
            for (auto i : buf.channel(0)) {
                channel0 = i;
                t.Fill();
                ++sample;
            }
        }
        t.Write();

        TTree integral {"integral", "integral"};
        int int_channel0 {0};
        integral.Branch("channel0", &int_channel0, "channel0/I");

        LOG_INFO("MAIN", "Measuring 500000 integrated pulses");
        for (int i = 0; i < 500000; ++i) {
            master->wait_for_irq();
            adc.read_pulse(buf);
            int_channel0 = -buf.integrate(0, {1340, 1400});
            integral.Fill();
        }
        integral.Write();

        f.Close();

    } catch (exception& e) {
        LOG_ERROR(e.type(), e.what());
        return 1;
    } catch (std::exception& e ) {
        LOG_ERROR("std::exception", e.what());
        return -1;
    }

    LOG_INFO("MAIN", "All done!");
    return 0;
}
