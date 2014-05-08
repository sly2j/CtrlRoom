#ifndef CTRLROOM_VME_MASTER_LOADED
#define CTRLROOM_VME_MASTER_LOADED

#include <ctrlroom/board.hpp>
#include <ctrlroom/vme/vme64.hpp>
#include <ctrlroom/vme/master/block_transfer.hpp>
#include <ctrlroom/util/logger.hpp>
#include <string>
#include <type_traits>

// TODO:
//  * implement IRQs
//  * implement RMW cycle
//  * implement ADO cycle
//  * implement lock and/or ADOH cycle
//  * design/implement FIFO block transfer support

// call VME_FRIEND_MASTER(master_type) from within classes that implement tha
// master interface to properly give the generic master
// (and block transfer implentation)
#ifndef VME_FRIEND_MASTER
#error VME_FRIEND_MASTER has to be defined (in vme/master/block_transfer.hpp)
#endif

namespace ctrlroom {
namespace vme {

class error;
class bus_error;
class comm_error;
class invalid_parameter;
class timeout_error;

// CRTP parent template for master modules.
// Implements the generic master interface.
// Derived implentations only have to define the necessary read/write/...
// functions (defined as protected members in the parent template)
// Master module implementations should friend the parent master for the
// CRTP to work, as the explicit read/write/... calls themselves are not part
// of the external interface (the generic read/write functions defined below
// are).
// CONFIGURATION FILE OPTIONS:
//      * Link index: <identifier>.linkIndex (cf. LINK_INDEX_KEY)
//      * board index: <identifier>.boardIndex (cf. BOARD_INDEX_KEY)
//      * IRQ: <identifier>.IRQ ([IRQ1, IRQ2], ...)
// optional
//      * TIMEOUT (in [ms]): <id>.timeout (defaults to 1000)
template <class MasterImpl> class master : public board {
public:
  constexpr static const char *LINK_INDEX_KEY{ "linkIndex" };
  constexpr static const char *BOARD_INDEX_KEY{ "boardIndex" };
  constexpr static const char *IRQ_KEY{ "IRQ" };
  constexpr static const char *TIMEOUT_KEY{ "timeout" };

  constexpr static size_t DEFAULT_TIMEOUT{ 1000 }; // in [ms]

  using base_type = board;
  using master_type = MasterImpl;

  master(const std::string &identifier, const ptree &settings);
  ~master();

  const std::string &name() const { return name_; }

  // wait for the next IRQ
  void wait_for_irq() const;
  // timeout is the configured value (<id>.timeout), unless explicitly
  // specified. This is only for expert usage.
  void wait_for_irq(size_t timeout) const;

  // READ a single value from <address> to <val> for transfer mode
  // D08_*, D16 or D32
  // returns the number of transactions (i.e., 1 if all went well)
  template <addressing_mode A, transfer_mode D,
            class = typename std::enable_if<!is_multiplexed<D>::value>::type>
  size_t read(const typename address_spec<A>::ptr_type address,
              typename transfer_spec<D>::value_type &val) const;
  // READ a block of values from <address> to <vals>
  // for all different types of block transfer (deduced from A and D)
  //
  // Returns the number of transaction Nt, where 0 <= Nt <= N.
  // Nt should be equal to N except when too many transactions
  // were requested. This case should be handled by the caller.
  template <addressing_mode A, transfer_mode D, class IntType, size_t N>
  size_t read(const typename address_spec<A>::ptr_type address,
              std::array<IntType, N> &vals) const;
  // WRITE a single value from <val> to <address> for transfer mode
  // D08_*, D16 or D32
  // returns the number of transactions (i.e., 1 if all went well)
  template <addressing_mode A, transfer_mode D,
            class = typename std::enable_if<!is_multiplexed<D>::value>::type>
  size_t write(const typename address_spec<A>::ptr_type address,
               typename transfer_spec<D>::value_type &val) const;
  // WRITE a block of values from <vals> to <address>
  // for all different types of block transfer (deduced from A and D)
  //
  // Returns the number of transaction Nt, where 0 <= Nt <= N.
  // Nt should be equal to N except when too many transactions
  // were requested. This case should be handled by the caller.
  template <addressing_mode A, transfer_mode D, class IntType, size_t N>
  size_t write(const typename address_spec<A>::ptr_type address,
               std::array<IntType, N> &vals) const;

  vme::error error(const std::string &msg) const;
  vme::bus_error bus_error(const std::string &msg) const;
  vme::comm_error comm_error(const std::string &msg) const;
  vme::invalid_parameter invalid_parameter(const std::string &msg) const;
  vme::timeout_error timeout_error(const std::string &msg) const;

protected:
  // READ/WRITE placeholder functions, to be replaced in MasterImpl if
  // the functionality is needed. Placeholders will generate a compilation
  // error if instantiated.
  // SINGLE
  template <addressing_mode A, transfer_mode D>
  size_t read_single(const typename address_spec<A>::ptr_type address,
                     typename transfer_spec<D>::ptr_type val) const;
  template <addressing_mode A, transfer_mode D>
  size_t write_single(const typename address_spec<A>::ptr_type address,
                      typename transfer_spec<D>::ptr_type val) const;
  // BLT
  template <addressing_mode A, transfer_mode D>
  size_t read_blt(const typename address_spec<A>::ptr_type address,
                  typename transfer_spec<D>::ptr_type buf,
                  size_t n_requests) const;
  template <addressing_mode A, transfer_mode D>
  size_t write_blt(const typename address_spec<A>::ptr_type address,
                   typename transfer_spec<D>::ptr_type buf,
                   size_t n_requests) const;
  // MD32
  template <addressing_mode A>
  size_t read_md32(const typename address_spec<A>::ptr_type address,
                   transfer_spec<transfer_mode::MD32>::ptr_type buf,
                   size_t n_requests) const;
  template <addressing_mode A>
  size_t write_md32(const typename address_spec<A>::ptr_type address,
                    transfer_spec<transfer_mode::MD32>::ptr_type buf,
                    size_t n_requests) const;
  // MBLT
  template <addressing_mode A>
  size_t read_mblt(const typename address_spec<A>::ptr_type address,
                   transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                   size_t n_requests) const;
  template <addressing_mode A>
  size_t write_mblt(const typename address_spec<A>::ptr_type address,
                    transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                    size_t n_requests) const;
  // 2eVME (3U)
  template <addressing_mode A>
  size_t read_2evme3(const typename address_spec<A>::ptr_type address,
                     transfer_spec<transfer_mode::U3_2eVME>::ptr_type buf,
                     size_t n_requests) const;
  template <addressing_mode A>
  size_t write_2evme3(const typename address_spec<A>::ptr_type address,
                      transfer_spec<transfer_mode::U3_2eVME>::ptr_type buf,
                      size_t n_requests) const;
  // 2eVME (6U)
  template <addressing_mode A>
  size_t read_2evme6(const typename address_spec<A>::ptr_type address,
                     transfer_spec<transfer_mode::U6_2eVME>::ptr_type buf,
                     size_t n_requests) const;
  template <addressing_mode A>
  size_t write_2evme6(const typename address_spec<A>::ptr_type address,
                      transfer_spec<transfer_mode::U6_2eVME>::ptr_type buf,
                      size_t n_requests) const;

  const short link_index_;
  const short board_index_;
  const std::vector<irq_level> irq_;
  const unsigned timeout_; // timeout level in [ms]

private:
  master_type &impl() { return static_cast<master_type &>(*this); }
  const master_type &impl() const {
    return static_cast<const master_type &>(*this);
  }

  // block transfer dispatchers (from master/block_transfer.hpp)
  template <addressing_mode A, transfer_mode D>
  friend struct master_impl::dispatch_read;
  template <addressing_mode A, transfer_mode D>
  friend struct master_impl::dispatch_write;

  // DRY block transfer implementation, used by both
  // ::read() and ::write()
  // (distinguished through different block transfer dispatchers).
  template <addressing_mode A, transfer_mode D, class IntType, size_t N,
            template <addressing_mode, transfer_mode> class Dispatcher>
  size_t block_transfer(const typename address_spec<A>::ptr_type address,
                        std::array<IntType, N> &vals) const;

  // DRY helper function for the various ::error methods
  template <class Error> Error error_helper(const std::string &msg) const {
    return { name_, link_index_, board_index_, msg };
  }
};
}
}

////////////////////////////////////////////////////////////////////////////////
// implementation: exceptions
////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
namespace vme {
class error : public ctrlroom::exception {
public:
  error(const std::string &board_name, const short link_index,
        const short board_index, const std::string &msg,
        const std::string &type = "error");
};
class bus_error : public error {
public:
  bus_error(const std::string &board_name, const short link_index,
            const short board_index, const std::string &msg);
};
class comm_error : public error {
public:
  comm_error(const std::string &board_name, const short link_index,
             const short board_index, const std::string &msg);
};
class invalid_parameter : public error {
public:
  invalid_parameter(const std::string &board_name, const short link_index,
                    const short board_index, const std::string &msg);
};
class timeout_error : public error {
public:
  timeout_error(const std::string &board_name, const short link_index,
                const short board_index, const std::string &msg);
};
}
}
////////////////////////////////////////////////////////////////////////////////
// implementation: master
////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
namespace vme {

// IRQ translator is defined in master.cpp
extern const translation_map<irq_level> IRQ_TRANSLATOR;

// constructor
template <class MasterImpl>
master<MasterImpl>::master(const std::string &identifier, const ptree &settings)
    : base_type{ identifier, settings },
      link_index_{ conf_.get<short>(LINK_INDEX_KEY) },
      board_index_{ conf_.get<short>(BOARD_INDEX_KEY) },
      irq_{ conf_.get_vector<irq_level>(IRQ_KEY, IRQ_TRANSLATOR) },
      timeout_{ conf_.get<unsigned>(TIMEOUT_KEY, DEFAULT_TIMEOUT) } {
  LOG_INFO(name(), "Initializing master module");
  if (timeout_ == 0) {
    throw conf_.value_error(TIMEOUT_KEY, std::to_string(timeout_));
  }
}

template <class MasterImpl> master<MasterImpl>::~master() {
  LOG_INFO(name(), "Releasing VME handle");
}

// read (main calls)
template <class MasterImpl>
template <addressing_mode A, transfer_mode D, class>
size_t
master<MasterImpl>::read(const typename address_spec<A>::ptr_type address,
                         typename transfer_spec<D>::value_type &val) const {
  return impl().template read_single<A, D>(address, &val);
}
template <class MasterImpl>
template <addressing_mode A, transfer_mode D, class IntType, size_t N>
size_t
master<MasterImpl>::read(const typename address_spec<A>::ptr_type address,
                         std::array<IntType, N> &vals) const {
  return block_transfer<A, D, IntType, N, master_impl::dispatch_read>(address,
                                                                      vals);
}

// write (main calls)
template <class MasterImpl>
template <addressing_mode A, transfer_mode D, class>
size_t
master<MasterImpl>::write(const typename address_spec<A>::ptr_type address,
                          typename transfer_spec<D>::value_type &val) const {
  return impl().template write_single<A, D>(address, &val);
}
template <class MasterImpl>
template <addressing_mode A, transfer_mode D, class IntType, size_t N>
size_t
master<MasterImpl>::write(const typename address_spec<A>::ptr_type address,
                          std::array<IntType, N> &vals) const {
  return block_transfer<A, D, IntType, N, master_impl::dispatch_write>(address,
                                                                       vals);
}

// block_transfer
template <class MasterImpl>
template <addressing_mode A, transfer_mode D, class IntType, size_t N,
          template <addressing_mode, transfer_mode> class Dispatcher>
size_t master<MasterImpl>::block_transfer(
    const typename address_spec<A>::ptr_type address,
    std::array<IntType, N> &vals) const {

  // number of elements to copy, in VME data width
  size_t n_to_copy{ N * sizeof(IntType) / transfer_spec<D>::WIDTH };

  // number of entries in the array
  size_t n_filled{ 0 };

  // loop over the necessary amount of block transfers,
  // taking into account the maximum allowed length block transfer lengths
  for (size_t n_blocks{ 1 + (n_to_copy - 1) / transfer_spec<D>::BLOCK_LENGTH };
       n_blocks > 0; --n_blocks) {

    // number of transactions for this block
    size_t n{ n_blocks == 1 ? n_to_copy : transfer_spec<D>::BLOCK_LENGTH };
    typename transfer_spec<D>::ptr_type vptr{
      reinterpret_cast<typename transfer_spec<D>::ptr_type>(&vals[n_filled])
    };

    // number of completed transactions in this call.
    size_t n_copied{ Dispatcher<A, D>::call(*this, address, vptr, n) };

    n_filled += n_copied * transfer_spec<D>::WIDTH / sizeof(IntType);
    n_to_copy -= n_copied;

    // handle the case where the block transfer ended prematurely
    if (!n_copied)
      break;
  }
  return n_filled;
}
// IRQ wait (default version)
template <class MasterImpl> void master<MasterImpl>::wait_for_irq() const {
  wait_for_irq(timeout_);
}

// exceptions
template <class MasterImpl>
vme::error master<MasterImpl>::error(const std::string &msg) const {
  return error_helper<vme::error>(msg);
}
template <class MasterImpl>
vme::bus_error master<MasterImpl>::bus_error(const std::string &msg) const {
  return error_helper<vme::bus_error>(msg);
}
template <class MasterImpl>
vme::comm_error master<MasterImpl>::comm_error(const std::string &msg) const {
  return error_helper<vme::comm_error>(msg);
}
template <class MasterImpl>
vme::invalid_parameter
master<MasterImpl>::invalid_parameter(const std::string &msg) const {
  return error_helper<vme::invalid_parameter>(msg);
}
template <class MasterImpl>
vme::timeout_error
master<MasterImpl>::timeout_error(const std::string &msg) const {
  return error_helper<vme::timeout_error>(msg);
}
}
}

#endif
