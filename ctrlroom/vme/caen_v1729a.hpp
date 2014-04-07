#ifndef CTRLROOM_VME_CAEN_V1729A_LOADED
#define CTRLROOM_VME_CAEN_V1729A_LOADED

#include <ctrlroom/vme/slave.hpp>

namespace ctrlroom {
    namespace vme {

        template <class Master, 
                        addressing_mode A = addressing_mode::A32, 
                        transfer_mode D = transfer_mode::MBLT>
            class caen_v1729a : public slave<Master, A> {
                public: 
                    using base_type = slave<Master>;

                    caen_v1729a(const std::string& identifier,
                                const ptree& settings,
                                std::shared_ptr<master_type>& master,
                                const address_type address)
                        : base_type{identifier, settings, master, address} {}

            }

    }
}



#endif
