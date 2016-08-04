#ifndef CTRLROOM_VME_CAEN_DISCRIMINATOR_SPEC_LOADED
#define CTRLROOM_VME_CAEN_DISCRIMINATOR_SPEC_LOADED

#include <ctrlroom/util/configuration.hpp>
#include <ctrlroom/vme/vme64.hpp>
#include <cstddef>

namespace ctrlroom {
namespace vme {
namespace caen_discriminator_impl {

enum class submodel {
  V895, // Leading edge discriminator
  V812  // CFD
};
// generic properties for CAEN VME discriminators. 

struct properties {
  static constexpr size_t N_CHANNELS{16};
  // maximum internal and external majority thresholds
  // (number of coincidences required for a "majority" signal")
  //  - internal mode: just for this board
  //  - external mode: when daisy-chaining multiple boards
  static constexpr size_t MAX_INTERNAL_MAJORITY_THRESHOLD{16};
  static constexpr size_t MAX_EXTERNAL_MAJORITY_THRESHOLD{20};
  // threshold range (in mV)
  static constexpr double MIN_THRESHOLD{-1};
  static constexpr double MAX_THRESHOLD{-255};
  // output pulse width range, depending on the discriminator board
  // NOTE: on all CAEN discriminators, the interpolation between minimum and
  // maximum width is non-linear (think rising exponential)
  static constexpr uint8_t MIN_OUTPUT_WIDTH{0};
  static constexpr uint8_t MAX_OUTPUT_WIDTH{255};
};

// extra properties that differ between the different discriminator boards
template <submodel M> struct extra_properties;
template <> struct extra_properties<submodel::V895> {
  // nothing here
};
template <> struct extra_properties<submodel::V812> {
  // dead time in ns
  static constexpr double MIN_DEADTIME {150};
  static constexpr double MAX_DEADTIME {2000};
};

// supported modes (addressing/write):
//      * A24/D16
//      * A32/D16
// board class derives from this class to generate a compile time
// error if an invalid mode is attempted (rather than waiting for
// bus errors)
template <addressing_mode A, transfer_mode D>
struct validate_mode {
  static_assert((A == addressing_mode::A24 && D == transfer_mode::D16) ||
                    (A == addressing_mode::A32 && D == transfer_mode::D16),
                "Invalid mode for discriminator board, "
                "only A24/D16 and A32/D16 supported");
};

// instructions the DISCRIMINATOR knows over VME (see manual for explanation)
template <addressing_mode A> struct instructions {
  using address_type = typename address_spec<A>::ptr_type;
  // write only registers
  static constexpr std::array<address_type, properties::N_CHANNELS> THRESHOLD{
      0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
      0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E};
  static constexpr address_type OUTPUT_WIDTH_0_7{0x40};
  static constexpr address_type OUTPUT_WIDTH_8_15{0x42};
  static constexpr address_type DEADTIME_0_7{0x44};  // V812 only
  static constexpr address_type DEADTIME_8_15{0x46}; // V812 only
  static constexpr address_type MAJORITY_THRESHOLD{0x48};
  static constexpr address_type PATTERN_INHIBITOR{0x4A};
  static constexpr address_type TEST_PULSE{0x4C};
  // read only registers
  static constexpr address_type FIXED_CODE{0xFA};
  static constexpr address_type MODEL_TYPE{0xFC};
  static constexpr address_type SERIAL{0xFE};
};

// channel mask (used for inhibit mask)
constexpr const std::array<uint16_t, properties::N_CHANNELS> CHANNEL_MASK{
    0x1,   0x2,   0x4,   0x8,   0x10,   0x20,   0x40,   0x80,
    0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000};
}
}
}

#endif
