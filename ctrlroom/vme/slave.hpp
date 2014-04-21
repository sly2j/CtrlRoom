#ifndef CTRLROOM_VME_SLAVE_LOADED
#define CTRLROOM_VME_SLAVE_LOADED


#include <memory>
#include <ctrlroom/board.hpp>
#include <ctrlroom/vme/master.hpp>

#include <ctrlroom/util/assert.hpp>

namespace ctrlroom {
    namespace vme {

        template <class Master, 
                        addressing_mode A,
                        transfer_mode DSingle,
                        transfer_mode DBLT>
            class slave : public board {
                public:
                    constexpr static const char* ADDRESS_KEY {"address"};

                    using base_type = board;
                    using master_type = Master;
                    using address_type = typename address_spec<A>::ptr_type;
                    using single_data_type = 
                        typename transfer_spec<DSingle>::value_type;
                    using blt_data_type = 
                        typename transfer_spec<DBLT>::value_type;

                    slave(const std::string& identifier,
                          const ptree& settings,
                          std::shared_ptr<master_type>& master);

                    unsigned read(
                            address_type a, 
                            single_data_type& val) const {
                        return master_->template read<A, DSingle>(address_ + a, val);
                    }
                    unsigned write(
                            address_type a, 
                            single_data_type val) const {
                        return master_->template write<A, DSingle>(address_ + a, val);
                    }
                    template <class Integer, unsigned N>
                        unsigned read(
                                address_type a, 
                                std::array<Integer, N>& vals) const {
                            return master_->template read<A, DBLT, N>(address_ + a, vals);
                        }
                    template <class Integer, unsigned N>
                        unsigned write(
                                address_type a, 
                                std::array<Integer, N>& vals) const {
                            return master_->template write<A, DBLT, N>(address_ + a, vals);
                        }

                protected:
                    std::shared_ptr<master_type> master_;
                    const address_type address_;

            };

    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// implementation: slave
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {

        template <class Master, 
                        addressing_mode A, 
                        transfer_mode DSingle, 
                        transfer_mode DBLT>
            slave<Master, A, DSingle, DBLT>::slave(
                    const std::string& identifier,
                    const ptree& settings,
                    std::shared_ptr<master_type>& master)
                : base_type {identifier, settings}
                , master_ {master} 
                , address_ {conf_.get<address_type>(ADDRESS_KEY)} {
                    tassert(master, "Invalid pointer to master module");
                }

    }
}


#endif
