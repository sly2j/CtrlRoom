#ifndef CTRLROOM_VME_SLAVE_LOADED
#define CTRLROOM_VME_SLAVE_LOADED


#include <memory>
#include <ctrlroom/board.hpp>
#include <ctrlroom/vme/master.hpp>

namespace ctrlroom {
    namespace vme {

        template <class Master, addressing_mode A>
            class slave : public board {
                public:
                    using base_type = board;
                    using master_type = Master;
                    using address_type = address_spec<A>::ptr_type;

                    slave(const std::string& identifier,
                          const ptree& settings,
                          std::shared_ptr<master_type>& master,
                          const address_type address);

                protected:
                    std::shared_ptr<master_type> master_;
                    address_type address_;

            };

    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// implementation: slave
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {

        template <class Master>
            slave::slave(
                    const std::string& identifier,
                    const ptree& settings,
                    std::shared_ptr<master_type>& master,
                    address_type address)
                : base_type {identifier, settings}
                , master_ {master} 
                , address_ {address} {}

    }
}


#endif
