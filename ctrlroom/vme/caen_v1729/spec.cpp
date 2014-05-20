#include "spec.hpp"

namespace ctrlroom {
namespace vme {
namespace caen_v1729_impl {

const translation_map<trigger_type> TRIGGER_TYPE_TRANSLATOR{
    {"software", trigger_type::SOFTWARE},
    {"internal", trigger_type::INTERNAL},
    {"external", trigger_type::EXTERNAL},
    {"or", trigger_type::OR}};
const translation_map<trigger_settings> TRIGGER_SETTINGS_TRANSLATOR{
    {"rising", trigger_settings::RISING_EDGE},
    {"falling", trigger_settings::FALLING_EDGE},
    {"random", trigger_settings::RANDOM},
    {"enExtTrig", trigger_settings::EN_EXT_TRIG}};

const translation_map<channel> CHANNEL_TRANSLATOR{{"C0", channel::C0},
                                                  {"C1", channel::C1},
                                                  {"C2", channel::C2},
                                                  {"C3", channel::C3},
                                                  {"CALL", channel::CALL}};

const translation_map<sampling_frequency> SAMPLING_FREQUENCY_TRANSLATOR{
    {"2GHz", sampling_frequency::FS_2GHZ},
    {"1GHz", sampling_frequency::FS_1GHZ}};

const translation_map<channel_multiplexing> CHANNEL_MULTIPLEXING_TRANSLATOR{
    {"single", channel_multiplexing::C_SINGLE},
    {"duplex", channel_multiplexing::C_DUPLEX},
    {"quadruplex", channel_multiplexing::C_QUADRUPLEX}};

const translation_map<uint8_t> BINARY_TRANSLATOR{{"false", 0}, {"true", 1}};
}
}
}
