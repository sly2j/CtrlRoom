#ifndef CTRLROOM_VME_VME64_LOADED
#define CTRLROOM_VME_VME64_LOADED

#include <cstdint>
#include <type_traits>

// TODO:
//  * implement 2eVME support for 3U modules

////////////////////////////////////////////////////////////////////////////////
// VME64/VME64x specification: addressing/transfer mode and utility functions
////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
namespace vme {
// adressing modes (VME64 standard)
// NOTES:
//      * A24 masters compatible with A16
//      * A32 masters compatible with A16 and A24
//      * A40 masters compatible with A16 and A24 (A32 not needed)
//      * A64 masters compatible with A16, A24 and A32 (A40 not needed)
enum class addressing_mode {
  A16, // short I/O addressing
  A24, // standard addressing
  A32, // extended addressing
  A40, // primarely used for 3U modules
  A64  // long addressing
};

// data transfer modes (VME64/VME64x standard)
enum class transfer_mode {
  DISABLED, // transfer mode disabled
  D08_O,    // (slave only) 8-bit transfer at odd addresses
  D08_EO,   // 8-bit transfer
  D16,      // 16-bit transfer
  D32,      // 32-bit transfer (6U only)
  MD32,     // 32-bit multiplexed transfer (3U and 6U)
  MBLT,     // 64-bit multiplexed transfer (6U only)
  U3_2eVME, // 64-bit 2-edge VME for 3U (VME64x)
  U6_2eVME  // 64-bit 2-edge VME for 6U (VME64x)
};

// interrupt levels (implemented as bit masks)
enum class irq_level {
  IRQ1 = 0x01, // interrupt level 1 (lowest)
  IRQ2 = 0x02,
  IRQ3 = 0x04,
  IRQ4 = 0x08,
  IRQ5 = 0x10,
  IRQ6 = 0x20,
  IRQ7 = 0x40 // interrupt level 7 (hightest)
};

// addressing spec
// contains address modifiers as well as extended address modifiers (XAM)
// Implementing the full VME64/VME64x spec, except for U3_2eVME.
// Interface: address_spec<A>::LCK
//                           ::DATA
//                           ::BLT
//                           ::MBLT
//                           ::PROG
//                           ::CS_CSR <- _24-bit_ control/status register space
//                           ::U3_2eVME
//                           ::U6_2eVME
//                           ::XAM_3U
//                           ::XAM_6U
//                           ::ptr_type
// NOTE: A function to automatically deduce the correct address modifiers
//       for block transfers, as well as a function to get the correct
//       XAM code for 2eVME transfers.
template <addressing_mode A> struct address_spec;
template <addressing_mode A, transfer_mode D>
constexpr uint32_t block_modifier();
template <addressing_mode A, transfer_mode D> constexpr uint32_t xam_code();

// To store the LSB and MSB of a 16-bit data register that is split in 2 8-bit
// registers
template <addressing_mode A> struct split_register;

// data transfer spec
// Interface: transfer_spec<D>::value_type      <- data type
//                            ::WIDTH           <- data width
//                            ::BLOCK_LENGTH    <- maximum block length
template <transfer_mode D> struct transfer_spec;

// data transfer trait to for a compile-time check for multiplexed modes
template <transfer_mode D> struct is_multiplexed;
}
}

////////////////////////////////////////////////////////////////////////////////
// VME64/VME64x definitions: address modifiers spec and data spec
////////////////////////////////////////////////////////////////////////////////
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
template <addressing_mode A> struct address_spec {};
// A16
template <> struct address_spec<addressing_mode::A16> {
  using ptr_type = uint16_t;
  static constexpr uint32_t LCK{0x2C};    // A16 lock command
  static constexpr uint32_t DATA{0x29};   // A16 (non-priv.) access
  static constexpr uint32_t DATA_S{0x2D}; // [obsolete] A16 (super.) access
};
// A24
template <> struct address_spec<addressing_mode::A24> {
  using ptr_type = uint32_t;
  static constexpr uint32_t LCK{0x35};    // A24 lock command
  static constexpr uint32_t DATA{0x39};   // A24 (non-priv.) data access
  static constexpr uint32_t BLT{0x3B};    // A24 (non-priv.) BLT
  static constexpr uint32_t MBLT{0x38};   // A24 (non-priv.) MBLT
  static constexpr uint32_t PROG{0x3A};   // A24 (non-priv.) prog. access
  static constexpr uint32_t DATA_S{0x3D}; // [obsolete] A24 (super.) data
                                          // access
  static constexpr uint32_t BLT_S{0x3F};  // [obsolete] A24 (super.) BLT
  static constexpr uint32_t MBLT_S{0x3C}; // [obsolete] A24 (super.) MBLT
  static constexpr uint32_t PROG_S{0x3E}; // [obsolete] A24 (super.) prog.
                                          // access
  static constexpr uint32_t CS_CSR{0x2F}; // 24-bit control/status register
                                          // space
};
// A32
template <> struct address_spec<addressing_mode::A32> {
  using ptr_type = uint32_t;
  static constexpr uint32_t LCK{0x05};      // A32 lock_command
  static constexpr uint32_t DATA{0x09};     // A32 (non-priv.) data access
  static constexpr uint32_t BLT{0x0B};      // A32 (non-priv.) BLT
  static constexpr uint32_t MBLT{0x08};     // A32 (non-priv.) MBLT
  static constexpr uint32_t PROG{0x0A};     // A32 (non-priv.) prog. access
  static constexpr uint32_t DATA_S{0x0D};   // [obsolete] A32 (super.) data
                                            // access
  static constexpr uint32_t BLT_S{0x0F};    // [obsolete] A32 (super.) BLT
  static constexpr uint32_t MBLT_S{0x0C};   // [obsolete] A32 (super.) MBLT
  static constexpr uint32_t PROG_S{0x0E};   // [obsolete] A32 (super.) prog.
                                            // access
  static constexpr uint32_t U3_2eVME{0x21}; // 2eVME for 3U bus modules
  static constexpr uint32_t U6_2eVME{0x20}; // 2eVME for 6U bus modules
  // static constexpr uint32_t XAM_U3 = TODO lookup value
  static constexpr uint32_t XAM_6U{0x01}; // A32 for 2eVME(6U)
};
// A40
template <> struct address_spec<addressing_mode::A40> {
  using ptr_type = uint64_t;
  static constexpr uint32_t LCK{0x35};      // A40 lock command
  static constexpr uint32_t DATA{0x34};     // A40 access
  static constexpr uint32_t MD32{0x37};     // A40 MD32
  static constexpr uint32_t U3_2eVME{0x21}; // 2eVME for 3U bus modules
  // static constexpr uint32_t XAM_3U = TODO lookup value
};
// A64
template <> struct address_spec<addressing_mode::A64> {
  using ptr_type = uint64_t;
  static constexpr uint32_t LCK{0x04};      // A64 lock command
  static constexpr uint32_t DATA{0x01};     // A64 data access
  static constexpr uint32_t BLT{0x03};      // A64 BLT
  static constexpr uint32_t MBLT{0x00};     // A64 MBLT
  static constexpr uint32_t U6_2eVME{0x20}; // 2eVME for 6U bus modules
  static constexpr uint32_t XAM_6U{0x02};   // A64 for 2eVME(6U)
};

namespace transfer_spec_impl {
template <class IntegerType, size_t BlockLength /* in bytes */> struct data {
  using value_type = IntegerType;
  using ptr_type = value_type*;
  static constexpr size_t WIDTH{sizeof(value_type)};
  static constexpr size_t BLOCK_LENGTH{BlockLength / sizeof(value_type)};
};
}

template <addressing_mode A> struct split_register {
  using address_type = typename address_spec<A>::ptr_type;
  const address_type LSB;
  const address_type MSB;
  constexpr split_register(address_type lower, address_type upper)
      : LSB{lower}, MSB{upper} {}
};

// transfer mode specification (VME64/VME64x)
// data value type      -> ::value_type
// data width           -> ::WIDTH
// (max) block length   -> ::BLOCK_LENGTH
template <transfer_mode D> struct transfer_spec {};
// DISABLED
template <>
struct transfer_spec<transfer_mode::DISABLED>
    : transfer_spec_impl::data<char, 0> {};
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
// NOTE: MBLT block length reduced from the 2048 from the spec
//       because the CAEN VX1718 starts huccuping when reading more
//       than 96 64-bit words (even though in theory it should be able
//       to split the blocks in hardware...)
template <>
struct transfer_spec<transfer_mode::MBLT>
    : transfer_spec_impl::data<int64_t, 96 * sizeof(int64_t)> {};
//                : transfer_spec_impl::data<int64_t, 2048> {};
// U3_2eVME
// TODO update when implemented!
template <>
struct transfer_spec<transfer_mode::U3_2eVME>
    : transfer_spec_impl::data<uint32_t, 0> {};
// U6_2eVME
template <>
struct transfer_spec<transfer_mode::U6_2eVME>
    : transfer_spec_impl::data<int64_t, 2048> {};
}
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of utility functions
////////////////////////////////////////////////////////////////////////////////

namespace ctrlroom {
namespace vme {
// helper function to correctly choose between the various
// types of block transfer modifiers
namespace am_impl {
template <addressing_mode A, transfer_mode D> struct block_mode {
  static constexpr uint32_t value{address_spec<A>::BLT};
};
template <addressing_mode A> struct block_mode<A, transfer_mode::MD32> {
  static constexpr uint32_t value{address_spec<A>::MD32};
};
template <addressing_mode A> struct block_mode<A, transfer_mode::MBLT> {
  static constexpr uint32_t value{address_spec<A>::MBLT};
};
template <addressing_mode A> struct block_mode<A, transfer_mode::U3_2eVME> {
  static constexpr uint32_t value{address_spec<A>::U3_2eVME};
};
template <addressing_mode A> struct block_mode<A, transfer_mode::U6_2eVME> {
  static constexpr uint32_t value{address_spec<A>::U6_2eVME};
};
};
template <addressing_mode A, transfer_mode D>
constexpr uint32_t deduce_block_modifier() {
  return am_impl::block_mode<A, D>::value;
}

// xam_code()
namespace xam_impl {
template <addressing_mode A, transfer_mode D> struct xam_code {
  // dummy default value the VME64 transfers
  static constexpr uint32_t value{0};
};
template <addressing_mode A> struct xam_code<A, transfer_mode::U3_2eVME> {
  static constexpr uint32_t value{address_spec<A>::XAM_3U};
};
template <addressing_mode A> struct xam_code<A, transfer_mode::U6_2eVME> {
  static constexpr uint32_t value{address_spec<A>::XAM_6U};
};
}
template <addressing_mode A, transfer_mode D> constexpr uint32_t xam_code() {
  return xam_impl::xam_code<A, D>::value;
}

// is_multiplexed<>
template <transfer_mode D> struct is_multiplexed : std::false_type {};
template <> struct is_multiplexed<transfer_mode::MD32> : std::true_type {};
template <> struct is_multiplexed<transfer_mode::MBLT> : std::true_type {};
template <> struct is_multiplexed<transfer_mode::U3_2eVME> : std::true_type {};
template <> struct is_multiplexed<transfer_mode::U6_2eVME> : std::true_type {};
}
}

#endif
