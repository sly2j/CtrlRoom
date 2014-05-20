#ifndef CTRLROOM_VME_SLAVE_LOADED
#define CTRLROOM_VME_SLAVE_LOADED

#include <ctrlroom/board.hpp>
#include <ctrlroom/vme/master.hpp>

#include <ctrlroom/util/assert.hpp>
#include <ctrlroom/util/logger.hpp>

#include <boost/lexical_cast.hpp>

#include <memory>
#include <cstddef>
#include <array>

namespace ctrlroom {
namespace vme {

template <class Master, addressing_mode A, transfer_mode DSingle,
          transfer_mode DBLT>
class slave : public board {
public:
  constexpr static const char* ADDRESS_KEY{"address"};

  using base_type = board;
  using master_type = Master;
  using address_type = typename address_spec<A>::ptr_type;
  using single_data_type = typename transfer_spec<DSingle>::value_type;
  using blt_data_type = typename transfer_spec<DBLT>::value_type;

  constexpr static addressing_mode addressing{A};
  constexpr static transfer_mode single_transfer{DSingle};
  constexpr static transfer_mode blt_transfer{DBLT};

  slave(const std::string& identifier, const ptree& settings,
        std::shared_ptr<master_type>& master);
  ~slave();

  size_t read(address_type a, single_data_type& val) const {
    return master_->template read<A, DSingle>(address_ + a, val);
  }
  size_t write(address_type a, single_data_type val) const {
    return master_->template write<A, DSingle>(address_ + a, val);
  }
  template <class Integer, size_t N>
  size_t read(address_type a, std::array<Integer, N>& vals) const {
    return master_->template read<A, DBLT>(address_ + a, vals);
  }
  template <class Integer, size_t N>
  size_t write(address_type a, std::array<Integer, N>& vals) const {
    return master_->template write<A, DBLT>(address_ + a, vals);
  }

protected:
  std::shared_ptr<master_type> master_;
  const address_type address_;
};
}
}

////////////////////////////////////////////////////////////////////////////////
// implementation: slave
////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
namespace vme {

template <class Master, addressing_mode A, transfer_mode DSingle,
          transfer_mode DBLT>
slave<Master, A, DSingle, DBLT>::slave(const std::string& identifier,
                                       const ptree& settings,
                                       std::shared_ptr<master_type>& master)
    : base_type{identifier, settings}
    , master_{master}
    , address_{static_cast<address_type>(
          std::stoll(conf_.get<std::string>(ADDRESS_KEY), nullptr, 0))} {
  tassert(master, "Invalid pointer to master module");
  LOG_INFO(name(), "Initializing slave module");
}
template <class Master, addressing_mode A, transfer_mode DSingle,
          transfer_mode DBLT>
slave<Master, A, DSingle, DBLT>::~slave() {
  LOG_INFO(name(), "Releasing control");
}
}
}

#endif
