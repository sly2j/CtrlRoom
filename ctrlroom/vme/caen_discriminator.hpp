#ifndef CTRLROOM_VME_CAEN_DISCRIMINATOR_LOADED
#define CTRLROOM_VME_CAEN_DISCRIMINATOR_LOADED

// TODO implement deadtime for the V812 board,
// currently V812 and V895 are identical!
//
// TODO implement majority threshold for external mode to allow daisy-chaining
// boards

#include <ctrlroom/vme/caen_discriminator/spec.hpp>
#include <ctrlroom/vme/slave.hpp>

#include <ctrlroom/util/assert.hpp>
#include <ctrlroom/util/exception.hpp>
#include <ctrlroom/util/logger.hpp>
#include <ctrlroom/util/mixin.hpp>
#include <ctrlroom/util/io/array.hpp>

#include <algorithm>
#include <array>
#include <limits>
#include <string>
#include <memory>
#include <cmath>
#include <fstream>

namespace ctrlroom {
namespace vme {
namespace caen_discriminator_impl {

// Generic implementation of the DISCRIMINATOR board
// specific typedefs for particular boards are provided below
// (in the main ctrlroom::vme namespace)
// Supported modes (addressing, read/write):
//      * A24/D16
//      * A32/D16
//      * A32/D32
// CONFIGURATION FILE OPTIONS:
// TODO
//      * Trigger type: <id>.triggerType (internal, external, ...)
//      * Trigger settings: <id>.triggerSettings ([rising, ...])
//      * Trigger threshold: <id>.triggerThreshold (-1000mV...1000mV)
//      * PRETRIG: <id>.pretrig (MIN, ... 0xFFFF)
//      * POSTTRIG: <id>.postTrig (7, ... 0xFFFF)
//      * sampling freq.: <id>.samplingFrequency (2GHz, 1GHz)
//  optional EXPERT USAGE ONLY (will cause calibrations to hang!!!)
//  (default to true)
//      * IRQ: <id>.enableIRQ (true, false)
//      * auto restart acquisition: <id>.autoRestartAcq (true, false)
//  optional, default to [ALL]
//      * Trigger channel source: <id>.triggerChannelSource
// ([CH0,CH1,CH2,CH3,ALL])
// TODO
template <class Master, submodel M, addressing_mode A,
          transfer_mode D = transfer_mode::D16>
class board : public slave<Master, A, D>,
              public properties,
              public extra_properties<M>,
              public validate_mode<A, D> {
public:
  static constexpr const char* THRESHOLD_KEY{"threshold"};
  static constexpr const char* CHANNEL_KEY_ROOT{
      "CH"}; // channel key is appended with %02i channel number
  static constexpr const char* OUTPUT_WIDTH_KEY{"width"};
  static constexpr const char* DEADTIME_KEY{"deadtime"};
  // optional (defaults to 1)
  static constexpr const char* MAJORITY_KEY{"coincidence"};

  using base_type = slave<Master, A, D>;
  using master_type = Master;
  using instructions = caen_discriminator_impl::instructions<A>;
  using data_type = typename base_type::single_data_type;
  using address_type = typename base_type::address_type;
  using base_type::name;
  using base_type::conf;
  using base_type::write;

  board(const std::string& identifier, const ptree& settings,
        std::shared_ptr<master_type>& master);

  // send a test pulse to the board
  void send_test_pulse();
};
}

// actual discriminator aliases
template <class Master, addressing_mode A, transfer_mode D = transfer_mode::D16>
using caen_v812 = caen_discriminator_impl::board<
    Master, caen_discriminator_impl::submodel::V812, A, D>;
template <class Master, addressing_mode A, transfer_mode D = transfer_mode::D16>
using caen_v895 = caen_discriminator_impl::board<
    Master, caen_discriminator_impl::submodel::V895, A, D>;
}
}

////////////////////////////////////////////////////////////////////////////////
// implementation: discriminator board
////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
namespace vme {
namespace caen_discriminator_impl {

template <class Master, submodel M, addressing_mode A, transfer_mode D>
board<Master, M, A, D>::board(const std::string& identifier,
                              const ptree& settings,
                              std::shared_ptr<Master>& master)
    : base_type{identifier, settings, master} {
  // set the trigger thresholds with matching inhibit mask
  LOG_JUNK(name(), "Setting the trigger thresholds");
  data_type inhibit_pattern = 0x0; // default: everything inhibited
  for (int ich = 0; ich < N_CHANNELS; ++ich) {
    char chkey[1024];
    snprintf(chkey, 2014, "%s.%s%02i", THRESHOLD_KEY, CHANNEL_KEY_ROOT, ich);
    const auto pthresh = conf().template get_optional<double>(chkey);
    if (pthresh) {
      // remember, thresholds are negative numbers!
      if (*pthresh < MAX_THRESHOLD || *pthresh > MIN_THRESHOLD) {
        throw conf().value_error(chkey, std::to_string(*pthresh));
      }
      LOG_JUNK(name(), "Threshold for channel " + std::to_string(ich) +
                           " set to " + std::to_string(*pthresh) + "mV");
      // convert to a 16 bit pattern
      const data_type thresh = static_cast<data_type>(fabs(*pthresh));
      // set the threshold on the board
      const auto th = instructions::THRESHOLD;
      std::cout << ich << " " << th[ich] << " " << thresh << std::endl;
      write(th[ich], thresh);
      // update the bitmask
      inhibit_pattern |= CHANNEL_MASK[ich];
    }
  }
  // set the inhibit mask
  LOG_JUNK(name(), "Setting the trigger inhibit mask");
  std::cout << "inhibit_pattern: " << inhibit_pattern << std::endl;
  write(instructions::PATTERN_INHIBITOR, inhibit_pattern);

  // set the output signal width (trigger window)
  LOG_JUNK(name(), "Setting the trigger window");
  const data_type output_width = conf().template get<data_type>(OUTPUT_WIDTH_KEY);
  if (output_width > MAX_OUTPUT_WIDTH || output_width < MIN_OUTPUT_WIDTH) {
    throw conf().value_error(OUTPUT_WIDTH_KEY, std::to_string(output_width));
  }
  std::cout << "OW: " << output_width << std::endl;
  write(instructions::OUTPUT_WIDTH_0_7, output_width);
  write(instructions::OUTPUT_WIDTH_8_15, output_width);

  // set the majority threshold (~coincidence, defaults to 1)
  LOG_JUNK(name(), "Setting the majority threshold");
  const auto pmaj = conf().template get_optional<size_t>(MAJORITY_KEY);
  const data_type maj = (pmaj) ? *pmaj : 1;
  if (maj > MAX_INTERNAL_MAJORITY_THRESHOLD || maj < 1) {
    throw conf().value_error(MAJORITY_KEY, std::to_string(maj));
  }
  write(instructions::MAJORITY_THRESHOLD,
        static_cast<data_type>(round((maj * 50 - 25) / 4)));
  std::cout << static_cast<data_type>(round((maj * 50 - 25) / 4)) << std::endl;

      // all done!
      LOG_JUNK(name(), "board initialized");
}

template <class Master, submodel M, addressing_mode A, transfer_mode D>
void board<Master, M, A, D>::send_test_pulse() {
  LOG_JUNK(name(), "Sending a test pulse to the board");
  write(instructions::TEST_PULSE, 1);
}

} // ns caen_discriminator_impl
} // ns vme
} // ns ctrlroom

#endif
