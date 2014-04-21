#ifndef CTRLROOM_VME_CAEN_V1729A_SPEC_LOADED
#define CTRLROOM_VME_CAEN_V1729A_SPEC_LOADED

#include <ctrlroom/util/configuration.hpp>
#include <ctrlroom/vme/vme64.hpp>
#include <cstddef>

namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {

            enum class submodel {
                V1729,      // old (12-bit) version
                V1729A      // new (14-bit) version
            };
            // base properties for both the 12 and 14-bit boards
            // NOTES:
            //      * measurements are stored in a 2560-word buffer preceded
            //        by a 12-word header (16-bit wordsize)
            //      * words are only 12/14-bit precise, have to be masked
            //      * the first <MEMORY_DATA_SKIP> values in the circular buffer
            //        cannot be trusted
            //      * VERNIERS stored in words 4-7
            //      * 16 kB of data generated during the vernier calibration procedure
            //      * intrinsic posttrigger equal to 7, only POSTTRIG-7 should be set
            //        in the register, as both values are added
            //      * minimum PRETRIG values vary between 1GHz and 2GHz sampling
            //        frequencies (corresponding to 100MHz and 50MHz clock speeds)
            //      * calibrated data will be returned as _SIGNED_ 32-bit integers
            //        (after pedestal subtraction, similar to a scope
            struct properties {
                static constexpr size_t N_CHANNELS {4};  
                static constexpr size_t N_CELLS {128};
                static constexpr size_t ROWS_PER_CELL {20};
                static constexpr size_t MEMORY_HEADER_SIZE {12};
                static constexpr size_t MEMORY_DATA_SIZE {
                    N_CHANNELS * N_CELLS * ROWS_PER_CELL
                };
                static constexpr size_t MEMORY_SIZE {
                    MEMORY_HEADER_SIZE + MEMORY_DATA_SIZE
                };
                static constexpr size_t MEMORY_DATA_SKIP {40};
                static constexpr size_t MEMORY_VERNIER_INDEX {4};
                static constexpr size_t VERNIER_MEMORY_SIZE {16 * 1024};
                static constexpr size_t INTRINSIC_POSTTRIG {7};
                static constexpr uint16_t MIN_PRETRIG_2GHZ {10000};
                static constexpr uint16_t MIN_PRETRIG_1GHZ {5000};
                static constexpr float MAX_ABS_TRIGGER_THRESHOLD {1000.f};  // in mV

                using memory_type = std::array<uint16_t, MEMORY_SIZE>;
                using vernier_type = std::array<uint16_t, N_CHANNELS>;
                using value_type = int32_t;
            };
            // extra properties that differ between the 12-bit and 14-bit version:
            //      * memory precision of course is either 12 or 14 bit
            //      * this leads to a different bit-mask when reading the memory
            //      * also the BIT_MODE (bit 1 of the MODE_REGISTER) differs
            //        between both boards
            template <submodel M> struct extra_properties;
            template <> struct extra_properties<submodel::V1729> {
                static constexpr submodel SUBMODEL {submodel::V1729};
                static constexpr unsigned MEMORY_PRECISION {12};
                static constexpr uint16_t MEMORY_MASK {0xffff >> (16 - MEMORY_PRECISION)};
                static constexpr uint8_t BIT_MODE {0x0};
            };
            template <> struct extra_properties<submodel::V1729A> {
                static constexpr submodel SUBMODEL {submodel::V1729A};
                static constexpr unsigned MEMORY_PRECISION {14};
                static constexpr uint16_t MEMORY_MASK {0xffff >> (16 - MEMORY_PRECISION)};
                static constexpr uint8_t BIT_MODE {0x2};
            };

            // instructions the V1729 knows over VME (see manual for explanation)
            template <addressing_mode A>
                struct instructions {
                    using address_type = typename address_spec<A>::ptr_type;
                    using split_type = split_register<A>;

                    static constexpr address_type INTERRUPT {0x0000};
                    static constexpr address_type FP_FREQUENCY {0x0100};
                    static constexpr address_type FPGA_VERSION {0x0200};
                    static constexpr address_type MODE_REGISTER {0x0300};
                    static constexpr address_type FPGA_EVOLUTION {0x0400};
                    static constexpr address_type RESET {0x0800};
                    static constexpr address_type LOAD_TRIGGER_THRESHOLD_DAC {0x0900};
                    static constexpr address_type TRIGGER_THRESHOLD_DAC {0x0A00};
                    static constexpr address_type RAM_DATA {0x0D00};
                    static constexpr split_type   RAM_INT_ADD {0x0E00, 0x0F00};
                    static constexpr split_type   MAT_CTRL_REGISTER {0x1000, 0x1100};
                    static constexpr address_type START_ACQUISITION {0x1700};
                    static constexpr split_type   PRETRIG {0x1800, 0x1900};
                    static constexpr split_type   POSTTRIG {0x1A00, 0x1B00};
                    static constexpr address_type SOFTWARE_TRIGGER {0x1C00};
                    static constexpr address_type TRIGGER_TYPE {0x1D00};
                    static constexpr address_type TRIGGER_CHANNEL_SOURCE {0x1E00};
                    static constexpr address_type TRIG_REC {0x2000};
                    static constexpr address_type FAST_READ_MODES {0x2100};
                    static constexpr address_type NB_OF_COLS_TO_READ {0x2200};
                    static constexpr address_type CHANNEL_MASK {0x2300};
                    static constexpr address_type VALP_CP_REGISTER {0x2600};
                    static constexpr address_type VALI_CP_REGISTER {0x2700};
                    static constexpr std::array<address_type, 4> TRIGGER_THRESHOLD_DAC_CH {
                        0x2800, 0x2900, 0x2A00, 0x2B00
                    };
                    static constexpr address_type EEPROM_WRITE {0x2C00};
                    static constexpr address_type EEPROM_POLL {0x2D00};
                    static constexpr address_type EEPROM_READ {0x2E00};
                    static constexpr std::array<uint8_t, 3> EEPROM_KEY_SEQUENCE {0xAA, 0x55, 0xA0};
                    static constexpr address_type POST_STOP_LATENCY {0x3000};
                    static constexpr address_type POST_LATENCY_PRETRIG {0x3100};
                    static constexpr address_type NUMBER_OF_CHANNELS {0x3400};
                    static constexpr address_type RATE_REG {0x3800};
                    static constexpr split_type   TRIG_COUNT {0x3900, 0x3A00};
                    static constexpr split_type   TRIG_RATE {0x3B00, 0x3C00};
                    static constexpr address_type TRIG_COUNT_RATE_BLOCK {0x3D00};
                };

            // TRIGGER_TYPE bit pattern
            enum trigger_type : uint8_t {
                SOFTWARE = 0x0,         // software trigger
                INTERNAL = 0x1,         // internal discriminator (DAC threshold)
                EXTERNAL = 0x2,         // external trigger
                OR = 0x3                // "or" of SOFTWARE and INTERNAL
            };
            extern const translation_map<trigger_type> TRIGGER_TYPE_TRANSLATOR;

            // additional trigger settings (composed with TRIGGER_TYPE)
            enum trigger_settings : uint8_t {
                RISING_EDGE = 0x0,      // rising edge
                FALLING_EDGE = 0x4,     // falling edge
                RANDOM = 0x8,           // authorize random trigger
                DIRECT_EXTERNAL = 0x16, // use external trigger directly and without masking
                EN_EXT_TRIG = 0x64      // trigger has to be validated by the EN_EXT_TRIG input
            };
            extern const translation_map<trigger_settings> TRIGGER_SETTINGS_TRANSLATOR;

            // channel mask (used for various instructions)
            enum channel : uint8_t {
                C0 = 0x1,
                C1 = 0x2,
                C2 = 0x4,
                C3 = 0x8,
                CALL = 0xF   // shorthand for C0|C1|C2|C3
            };
            extern const translation_map<channel> CHANNEL_TRANSLATOR;

            // only support the native clock frequencies for now
            // integer code is the divider of 2GHz to get the
            // wanted frequency:
            //      2GHz = 2GHz / 1 ==> 1
            //      1GHz = 2GHz / 2 ==> 2
            enum sampling_frequency : uint8_t {
                FS_2GHZ = 0x1,
                FS_1GHZ = 0x2
            };
            extern const translation_map<sampling_frequency> 
                SAMPLING_FREQUENCY_TRANSLATOR;

            // multiplexing (number of channels used per channel)
            enum channel_multiplexing : uint8_t {
                C_SINGLE = 4,
                C_DUPLEX = 2,
                C_QUADRUPLEX = 1
            };
            extern const translation_map<channel_multiplexing> CHANNEL_MULTIPLEXING_TRANSLATOR;

            // utility translator (true/false)
            extern const translation_map<uint8_t> BINARY_TRANSLATOR;
        }
    }
}

#endif
