#ifndef CTRLROOM_VME_MASTER_BLOCK_TRANSFER_LOADED
#define CTRLROOM_VME_MASTER_BLOCK_TRANSFER_LOADED

#include <ctrlroom/vme/vme64.hpp>

#define VME_FRIEND_MASTER(master_type)                                         \
  template <addressing_mode A, transfer_mode D>                                \
  friend struct ctrlroom::vme::master_impl::dispatch_read;                     \
  template <addressing_mode A, transfer_mode D>                                \
  friend struct ctrlroom::vme::master_impl::dispatch_write;                    \
  friend master_type;

namespace ctrlroom {
namespace vme {
namespace master_impl {
// compile time "dispatch" of the correct block transfer function
// (BLT, MBLT, MD32, 2eVME) using partial specialization on the
// transfer_mode. Implements a static ::call method
// method that calls the relevant _blt, _md32, _mblt, _2evme3 or _23vme6
// method of the VME master module (through friendship)
template <addressing_mode A, transfer_mode D> struct dispatch_read;
template <addressing_mode A, transfer_mode D> struct dispatch_write;
}
}
}

////////////////////////////////////////////////////////////////////////////////
// Implementation: dispatch_read
////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
namespace vme {
namespace master_impl {
// general one: for blt
template <addressing_mode A, transfer_mode D> struct dispatch_read {
  template <class Master>
  static unsigned
  call(const Master &m, const typename address_spec<A>::ptr_type address,
       typename transfer_spec<D>::ptr_type vptr, const unsigned n_requests) {
    return m.impl().template read_blt<A, D>(address, vptr, n_requests);
  }
};
template <addressing_mode A, transfer_mode D> struct dispatch_write {
  template <class Master>
  static unsigned
  call(const Master &m, const typename address_spec<A>::ptr_type address,
       typename transfer_spec<D>::ptr_type vptr, const unsigned n_requests) {
    return m.impl().template write_blt<A, D>(address, vptr, n_requests);
  }
};
// specialization for md32
template <addressing_mode A> struct dispatch_read<A, transfer_mode::MD32> {
  template <class Master>
  static unsigned call(const Master &m,
                       const typename address_spec<A>::ptr_type address,
                       transfer_spec<transfer_mode::MD32>::ptr_type vptr,
                       const unsigned n_requests) {
    return m.impl().template read_md32<A>(address, vptr, n_requests);
  }
};
template <addressing_mode A> struct dispatch_write<A, transfer_mode::MD32> {
  template <class Master>
  static unsigned call(const Master &m,
                       const typename address_spec<A>::ptr_type address,
                       transfer_spec<transfer_mode::MD32>::ptr_type vptr,
                       const unsigned n_requests) {
    return m.impl().template write_md32<A>(address, vptr, n_requests);
  }
};
// specialization for mblt
template <addressing_mode A> struct dispatch_read<A, transfer_mode::MBLT> {
  template <class Master>
  static unsigned call(const Master &m,
                       const typename address_spec<A>::ptr_type address,
                       transfer_spec<transfer_mode::MBLT>::ptr_type vptr,
                       const unsigned n_requests) {
    return m.impl().template read_mblt<A>(address, vptr, n_requests);
  }
};
template <addressing_mode A> struct dispatch_write<A, transfer_mode::MBLT> {
  template <class Master>
  static unsigned call(const Master &m,
                       const typename address_spec<A>::ptr_type address,
                       transfer_spec<transfer_mode::MBLT>::ptr_type vptr,
                       const unsigned n_requests) {
    return m.impl().template write_mblt<A>(address, vptr, n_requests);
  }
};
// specialization for 2evme (3u)
template <addressing_mode A> struct dispatch_read<A, transfer_mode::U3_2eVME> {
  template <class Master>
  static unsigned call(const Master &m,
                       const typename address_spec<A>::ptr_type address,
                       transfer_spec<transfer_mode::U3_2eVME>::ptr_type vptr,
                       const unsigned n_requests) {
    return m.impl().template read_2evme3<A>(address, vptr, n_requests);
  }
};
template <addressing_mode A> struct dispatch_write<A, transfer_mode::U3_2eVME> {
  template <class Master>
  static unsigned call(const Master &m,
                       const typename address_spec<A>::ptr_type address,
                       transfer_spec<transfer_mode::U3_2eVME>::ptr_type vptr,
                       const unsigned n_requests) {
    return m.impl().template write_2evme3<A>(address, vptr, n_requests);
  }
};
// specialization for 2evme (6u)
template <addressing_mode A> struct dispatch_read<A, transfer_mode::U6_2eVME> {
  template <class Master>
  static unsigned call(const Master &m,
                       const typename address_spec<A>::ptr_type address,
                       transfer_spec<transfer_mode::U6_2eVME>::ptr_type vptr,
                       const unsigned n_requests) {
    return m.impl().template read_2evme6<A>(address, vptr, n_requests);
  }
};
template <addressing_mode A> struct dispatch_write<A, transfer_mode::U6_2eVME> {
  template <class Master>
  static unsigned call(const Master &m,
                       const typename address_spec<A>::ptr_type address,
                       transfer_spec<transfer_mode::U6_2eVME>::ptr_type vptr,
                       const unsigned n_requests) {
    return m.impl().template write_2evme6<A>(address, vptr, n_requests);
  }
};
}
}
}

#endif
