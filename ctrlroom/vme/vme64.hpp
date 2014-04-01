#ifndef CTRLROOM_VME_VME64_LOADED
#define CTRLROOM_VME_VME64_LOADED

#include <cstdint>

//////////////////////////////////////////////////////////////////////////////////////////
// VME64/VME64x specification: addressing/transfer mode and utility functions
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {
        // adressing modes (VME64 standard)
        // NOTES:
        //      * A24 masters compatible with A16
        //      * A32 masters compatible with A16 and A24
        //      * A40 masters compatible with A16 and A24 (A32 not needed)
        //      * A64 masters compatible with A16, A24 and A32 (A40 not needed)
        enum class addressing_mode {
            A16,            // short I/O addressing
            A24,            // standard addressing
            A32,            // extended addressing
            A40,            // primarely used for 3U modules
            A64             // long addressing
        };

        // data transfer modes (VME64/VME64x standard)
        enum class transfer_mode {
            D08_O,          // (slave only) 8-bit transfer at odd addresses
            D08_EO,         // 8-bit transfer
            D16,            // 16-bit transfer
            D32,            // 32-bit transfer (6U only)
            MD32,           // 32-bit multiplexed transfer (3U and 6U)
            MBLT,           // 64-bit multiplexed transfer (6U only)
            U3_2eVME,       // 64-bit 2-edge VME for 3U (VME64x)
            U6_2eVME        // 64-bit 2-edge VME for 6U (VME64x)
        };

        // address modifier utility functions
        //  * LCK modifiers
        template <addressing_mode A> constexpr unsigned lock_modifier();
        //  * DATA modifier
        template <addressing_mode A> constexpr unsigned data_modifier();
        //  * BLOCK TRANSFER modifiers (BLT, MBLT, MD32, 2eVME)
        template <addressing_mode A, transfer_mode D> constexpr unsigned block_modifier();
        //  * CS/CSR modifier (A24)
        constexpr unsigned cs_csr_modifier();

        // extended address modifier (XAM) utility function
        template <addressing_mode A, transfer_mode D> constexpr unsigned xam_code();

        // data transfer utility functions
        //  * get data width (in bytes)
        template <transfer_mode D> constexpr unsigned data_width();
        //  * get (maximum) block transfer length
        template <transfer_mode D> constexpr unsigned block_length();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// VME64/VME64x definitions: address modifiers spec and data spec
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {

        // Address specification (VME64/VME64x standard)
        // Includes address modifiers and extended address modifiers (XAM codes)
        // NOTES:
        //      * The supervisory/non-privileged mode distinction is obsolete
        //         -- they corresponeded to the 68000 microprocessor supervisor
        //            and user modes, an early form of memory management.
        //        It is recommended to use the non-privileged versions for 
        //        simplicity's sake
        //      * (SJJ) unsure what the purpose of the program access modifiers
        //              is, and if they are still relevant.
        template <addressing_mode A>
            struct address_spec {};
        // A16
        template <>
            struct address_spec<addressing_mode::A16> {
                enum class address_modifier
                    : int32_t {
                        LCK = 0x2C,     // A16 lock command
                        DATA = 0x29,    // A16 (non-privileged) access
                        DATA_S = 0x2D   // A16 (supervisory) access (obsolete)
                    };
                enum class extended_address_modifier 
                    : int32_t {};
            };
        // A24
        template <>
            struct address_spec<addressing_mode::A24> {
                enum class address_modifier
                    : int32_t {
                        LCK = 0x35,     // A24 lock command (LCK)
                        DATA = 0x39,    // A24 (non-privileged) data access
                        BLT = 0x3B,     // A24 (non-privileged) block transfer (BLT)
                        MBLT = 0x38,    // A24 (non-privileged) 64-bit block transfer (MBLT)
                        PROG = 0x3A,    // A24 (non-privileged) program access
                        DATA_S = 0x3D,  // [obsolete] A24 (supervisory) data access
                        BLT_S = 0x3F,   // [obsolete] A24 (supervisory) block transfer (BLT)
                        MBLT_S = 0x3C,  // [obsolete] A24 (supervisory) 64-bit block transfer (MBLT)
                        PROG_S = 0x3E,  // [obsolete] A24 (supervisory) program access

                        CS_CSR = 0x2F   // 24-bit control/status register space
                    };
                enum class extended_address_modifier
                    : int32_t {};
            };
        // A32
        template <>
            struct address_spec<addressing_mode::A32> {
                enum class address_modifier
                    : int32_t {
                        LCK = 0x05,     // A32 lock command (LCK)
                        DATA = 0x09,    // A32 (non-privileged) data access
                        BLT = 0x0B,     // A32 (non-privileged) block transfer (BLT)
                        MBLT = 0x08,    // A32 (non-privileged) 64-bit block transfer (MBLT)
                        PROG = 0x0A,    // A32 (non-privileged) program access
                        DATA_S = 0x0D,  // [obsolete] A32 (supervisory) data access
                        BLT_S = 0x0F,   // [obsolete] A32 (supervisory) block transfer (BLT)
                        MBLT_S = 0x0C,  // [obsolete] A32 (supervisory) 64-bit block transfer (MBLT)
                        PROG_S = 0x0E,  // [obsolete] A32 (supervisory) program access

                        U3_2eVME = 0x21,// 2eVME for 3U bus modules (set A32 address size in XAM)
                        U6_2eVME = 0x20,// 2eVME for 6U bus modules (set A32 address size in XAM)
                    };
                enum class extended_address_modifier
                    : int32_t {
                        // U3_2eVME = TODO lookup value
                        U6_2eVME = 0x01
                    };
            };
        // A40
        template <>
            struct address_spec<addressing_mode::A40> {
                enum class address_modifier
                    : int32_t {
                        LCK = 0x35,     // A40 lock command (LCK)
                        DATA = 0x34,    // A40 access
                        MD32 = 0x37,    // A40 block transfer (MD32)
                        U3_2eVME = 0x21 // 2eVME for 3U bus modules (set A40 address size in XAM)
                    };
                enum class extended_address_modifier
                    : int32_t {
                        // U3_2eVME = TODO lookup value
                    };
            };
        // A64
        template <>
            struct address_spec<addressing_mode::A64> {
                enum class address_modifier
                    : int32_t {
                        LCK = 0x04,     // A64 lock command (LCK)
                        DATA = 0x01,    // A64 single transfer access
                        BLT = 0x03,     // A64 block transfer (BLT)
                        MBLT = 0x00,    // A64 64-bit block transfer (MBLT)
                        U6_2eVME = 0x20 // 2eVME for 6U bus modules (set A64 address size in XAM)
                    };
                enum class extended_address_modifier
                    : int32_t {
                        U6_2eVME = 0x02
                    };
            };

        namespace transfer_spec_impl {
            template <class IntegerType, unsigned BlockLenght /* in bytes */>
            struct data {
                enum class data
                    : unsigned {
                        WIDTH = sizeof(IntegerType),
                        BLOCK_LENGTH = BlockLength / sizeof(IntegerType)
                    };
            };
        }

        // transfer mode specification (VME64/VME64x)
        template <transfer_mode D>
            struct transfer_spec {
                enum class data
                    : unsigned {
                        WIDTH,
                        BLOCK_LENGTH
                    };
            };
        // D08_O
        template <>
            struct transfer_spec<transfer_mode::D08_O>
                : transfer_spec_impl::data<int8_t, 256> {};
        // D08_EO
        template <>
            struct transfer_spec<transfer_mode::D08_EO>
                : transfer_spec_impl::data<int8_t, 256> {};
        // D16
        template <>
            struct transfer_spec<transfer_mode::D16>
                : transfer_spec_impl::data<int16_t, 256> {};
        // D32
        template <>
            struct transfer_spec<transfer_mode::D32>
                : transfer_spec_impl::data<int32_t, 256> {};
        // MD32
        template <>
            struct transfer_spec<transfer_mode::MD32>
                : transfer_spec_impl::data<int32_t, 256> {};
        // MBLT
        template <>
            struct transfer_spec<transfer_mode::MBLT>
                : transfer_spec_impl::data<int64_t, 2048> {};
        // U3_2eVME
        template <>
            struct transfer_spec<transfer_mode::U3_2eVME> {
                static_assert(false, "not implemented");
            };
        // U6_2eVME
        template <>
            struct transfer_spec<transfer_mode::U6_2eVME>
                : transfer_spec_impl::data<int64_t, 2048> {};

    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Implementation of utility functions
//////////////////////////////////////////////////////////////////////////////////////////

namespace ctrlroom {
    namespace vme {
        // *_modifier() functions
        namespace am_impl {
            // helper structs to correctly choose between the various
            // types of block transfer modifiers
            template <addressing_mode A, transfer_mode D> {
                struct block_mode {
                    constexpr int32_t value {address_spec<A>::address_modifier::BLT};
                };
            template <addressing_mode A>
                struct block_mode<A, transfer_mode::MD32> {
                    constexpr int32_t value {address_spec<A>::address_modifier::MD32};
                };
            template <addressing_mode A>
                struct block_mode<A, transfer_mode::MBLT> {
                    constexpr int32_t value {address_spec<A>::address_modifier::MBLT};
                };
            template <addressing_mode A>
                struct block_mode<A, transfer_mode::U3_2eVME> {
                    constexpr int32_t value {address_spec<A>::address_modifier::U3_2eVME};
                };
            template <addressing_mode A>
                struct block_mode<A, transfer_mode::U6_2eVME> {
                    constexpr int32_t value {address_spec<A>::address_modifier::U6_2eVME};
                };
        };
        template <addressing_mode A>
            constexpr int32_t lock_modifier() {
                return {address_spec<A>::address_modifier::LCK};
            }
        template <addressing_mode A>
            constexpr int32_t data_modifier() {
                return {address_spec<A>::address_modifier::DATA};
            }
        template <addressing_mode A, transfer_mode D>
            constexpr int32_t block_modifier() {
                return am_impl::block_mode<A, D>::value;
            }
        constexpr int32_t cs_csr_modifier() {
            return {address_spec<addressing_mode::A24>::address_modifier::CS_CSR};
        }

        // xam_code()
        namespace xam_impl {
            template <addressing_mode A, transfer_mode D> {
                struct xam_code {};
            template <addressing_mode A> {
                struct xam_code<A, transfer_mode::U3_2eVME> {
                    constexpr int32_t value {address_spec<A>::extended_address_modifier::U3_2eVME};
                };
            template <addressing_mode A> {
                struct xam_code<A, transfer_mode::U6_2eVME> {
                    constexpr int32_t value {address_spec<A>::extended_address_modifier::U6_2eVME};
                };
        }
        template <addressing_mode A, transfer_mode D>
            constexpr int32_t xam_code() {
                return xam_impl::xam_code<A, D>::value;
            }

        // data_width()
        template <transfer_mode D>
            constexpr unsigned data_width() {
                return {transfer_spec<D>::data::WIDTH};

        // block_length()
        template <transfer_mode D>
            constexpr unsigned block_length() {
                return {transfer_spec<D>::data::BLOCK_LENGTH};
    }
}

#endif
