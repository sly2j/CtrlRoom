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

#include <TFile.h>
#include <TTree.h>

using namespace ctrlroom;

using bridge_type = vme::caen_bridge;
using adc_type = vme::caen_v1729a<bridge_type, vme::addressing_mode::A32,
                                  vme::transfer_mode::D32,   // single
                                  vme::transfer_mode::MBLT>; // BLT

int main(int argc, char *argv[]) {

  global::logger.set_level(log_level::JUNK2);

  try {
    tassert(argc == 2, "One command line parameter required (output file)")

    ptree config;
    read_json("test.json", config);

    LOG_INFO("MAIN", "Obtaining bridge handle");
    std::shared_ptr<bridge_type> master{ new bridge_type{ "bridge", config } };
    LOG_INFO("MAIN", "Calibrating ADC pedestals");
    adc_type::measure_pedestal("ADC", config, master,
                               "/daq/pmtcoat/calibrations/001", 100);
    // adc_type::memory_type pedestal {0};
    LOG_INFO("MAIN", "Calibrating ADC verniers");
    adc_type::calibrate_verniers("ADC", config, master,
                                 "/daq/pmtcoat/calibrations/001");
    // std::pair<adc_type::vernier_type, adc_type::vernier_type> vernier {
    //    {8588, 8680, 8649, 8588},
    //    {10293, 10429, 10385, 10299}
    //};

    LOG_WARNING("MAIN", "Hookup your wires and press enter to continue...");
    std::cin.ignore();
    LOG_INFO("MAIN", "Obtaining ADC handle");
    adc_type adc{ "ADC", config, master, "/daq/pmtcoat/calibrations/001" };

    adc_type::buffer_type buf;

    TFile f{ argv[1], "recreate" };
    TTree t{ "ramp", "ramp" };
    int event{ 0 };
    int sample{ 0 };
    int channel0{ 0 };
    int channel1{ 0 };
    int channel2{ 0 };
    int channel3{ 0 };
    t.Branch("event", &event, "event/I");
    t.Branch("sample", &sample, "sample/I");
    t.Branch("channel0", &channel0, "channel0/I");
    t.Branch("channel1", &channel1, "channel1/I");
    t.Branch("channel2", &channel2, "channel2/I");
    t.Branch("channel3", &channel3, "channel3/I");
    LOG_INFO("MAIN", "Measuring 100 pulses");
    for (int i = 0; i < 100; ++i) {
      master->wait_for_irq();
      adc.read_pulse(buf);
      for (unsigned j{ 0 }; j < buf.size(); ++j) {
        event = i;
        sample = j;
        channel0 = buf.get(0, j);
        channel1 = buf.get(1, j);
        channel2 = buf.get(2, j);
        channel3 = buf.get(3, j);
        t.Fill();
      }
    }
    t.Write();

    TTree integral{ "integral", "integral" };
    int int_channel0{ 0 };
    int int_channel1{ 0 };
    int int_channel2{ 0 };
    int int_channel3{ 0 };
    integral.Branch("channel0", &int_channel0, "channel0/I");
    integral.Branch("channel1", &int_channel1, "channel1/I");
    integral.Branch("channel2", &int_channel2, "channel2/I");
    integral.Branch("channel3", &int_channel3, "channel3/I");

    LOG_INFO("MAIN", "Measuring 50000 integrated pulses");
    for (int i = 0; i < 50000; ++i) {
      master->wait_for_irq();
      adc.read_pulse(buf);
      int_channel0 = -buf.integrate(0, { 1340, 1400 });
      int_channel1 = -buf.integrate(1, { 1340, 1400 });
      int_channel2 = -buf.integrate(2, { 1340, 1400 });
      int_channel3 = -buf.integrate(3, { 1340, 1400 });
      integral.Fill();
    }
    integral.Write();

    f.Close();
  }
  catch (exception &e) {
    LOG_ERROR(e.type(), e.what());
    return 1;
  }
  catch (std::exception &e) {
    LOG_ERROR("std::exception", e.what());
    return -1;
  }

  LOG_INFO("MAIN", "All done!");
  return 0;
}
