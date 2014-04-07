#ifndef CTRLROOM_VME_CAEN_BRIDGE_LOADED
#define CTRLROOM_VME_CAEN_BRIDGE_LOADED

#include <ctrlroom/util/exception.hpp>
#include <ctrlroom/vme/master.hpp>

#include <CAENVMElib.h>
#include <cstddef>
#include <string>


// Throw a vme::error if a problem was encountered.
// macro instead of inline function to avoid unnecessary 
// evaluation of msg in tight loops
#define HANDLE_CAEN_ERROR(err, msg) \
    if ((err)) {throw decode_error((err), (msg));}

namespace ctrlroom {
    namespace vme {

        class caen_bridge
            : public vme::master<caen_bridge> {
                public:
                    using base_type = vme::master<caen_bridge>;

                    caen_bridge(
                            const std::string& identifier,
                            const ptree& settings);

                    ~caen_bridge();

                    // wait for the next IRQ
                    void wait_for_irq() const;


                protected:
                    // Single
                    template <addressing_mode A, transfer_mode D>
                        unsigned read_single(
                                const typename address_spec<A>::ptr_type address,
                                typename transfer_spec<D>::ptr_type val) const;
                    template <addressing_mode A, transfer_mode D>
                        unsigned write_single(
                                const typename address_spec<A>::ptr_type address,
                                typename transfer_spec<D>::ptr_type val) const;
                    // BLT
                    template <addressing_mode A, transfer_mode D>
                        unsigned read_blt(
                                const typename address_spec<A>::ptr_type address,
                                typename transfer_spec<D>::ptr_type buf,
                                unsigned n_requests) const;
                    template <addressing_mode A, transfer_mode D>
                        unsigned write_blt(
                                const typename address_spec<A>::ptr_type address,
                                typename transfer_spec<D>::ptr_type buf,
                                unsigned n_requests) const;
                    // MBLT
                    template <addressing_mode A>
                        unsigned read_mblt(
                                const typename address_spec<A>::ptr_type address,
                                transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                                unsigned n_requests) const;
                    template <addressing_mode A>
                        unsigned write_mblt(
                                const typename address_spec<A>::ptr_type address,
                                transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                                unsigned n_requests) const;
                private:
                    void setup();
                    void init();
                    void end();

                    // calculate the IRQ mask from base_type::irq_
                    uint32_t calc_irq_mask() const;

                    vme::error decode_error(
                            const CVErrorCodes err,
                            const std::string& msg) const;

                    int32_t handle_;
                    const CVBoardTypes model_;
                    uint32_t irq_mask_;
        };
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// implementation: caen_bridge
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {

        template <addressing_mode A, transfer_mode D>
            unsigned caen_bridge::read_single(
                    const typename address_spec<A>::ptr_type address,
                    typename transfer_spec<D>::ptr_type val) const {

                CVErrorCodes err {
                    CAENVME_ReadCycle(
                            handle_, 
                            address, 
                            val,
                            address_spec<A>::DATA,
                            transfer_spec<D>::WIDTH)
                };
                HANDLE_CAEN_ERROR(err, "ReadCycle call failed");
                return {1};
            }
        template <addressing_mode A, transfer_mode D>
            unsigned caen_bridge::write_single(
                    const typename address_spec<A>::ptr_type address,
                    typename transfer_spec<D>::ptr_type val) const {

                CVErrorCodes err {
                    CAENVME_WriteCycle(
                            handle_, 
                            address, 
                            val,
                            address_spec<A>::DATA,
                            transfer_spec<D>::WIDTH)
                };
                HANDLE_CAEN_ERROR(err, "ReadCycle call failed");
                return {1};
            }

        template <addressing_mode A, transfer_mode D>
            unsigned caen_bridge::read_blt(
                    const typename address_spec<A>::ptr_type address,
                    typename transfer_spec<D>::ptr_type buf,
                    unsigned n_requests) const {
                int n_read {0};
                CVErrorCodes err {
                    CAENVME_BLTReadCycle(
                            handle_,
                            address,
                            buf,
                            n_requests,
                            address_spec<A>::BLT,
                            transfer_spec<D>::WIDTH,
                            &n_read)
                };
                HANDLE_CAEN_ERROR(err, "BLTReadCycle failed");
                return static_cast<unsigned>(n_read);
            }
        template <addressing_mode A, transfer_mode D>
            unsigned caen_bridge::write_blt(
                    const typename address_spec<A>::ptr_type address,
                    typename transfer_spec<D>::ptr_type buf,
                    unsigned n_requests) const {
                int n_written {0};
                CVErrorCodes err {
                    CAENVME_BLTWriteCycle(
                            handle_,
                            address,
                            buf,
                            n_requests,
                            address_spec<A>::BLT,
                            transfer_spec<D>::WIDTH,
                            &n_written)
                };
                HANDLE_CAEN_ERROR(err, "BLTWriteCycle failed");
                return static_cast<unsigned>(n_written);
            }
        template <addressing_mode A>
            unsigned caen_bridge::read_mblt(
                    const typename address_spec<A>::ptr_type address,
                    transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                    unsigned n_requests) const {
                int n_read {0};
                CVErrorCodes err {
                    CAENVME_MBLTReadCycle(
                            handle_,
                            address,
                            buf,
                            n_requests,
                            address_spec<A>::MBLT,
                            &n_read)
                };
                HANDLE_CAEN_ERROR(err, "MBLTReadCycle failed");
                return static_cast<unsigned>(n_read);
            }
        template <addressing_mode A>
            unsigned caen_bridge::write_mblt(
                    const typename address_spec<A>::ptr_type address,
                    transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                    unsigned n_requests) const {
                int n_written {0};
                CVErrorCodes err {
                    CAENVME_MBLTWriteCycle(
                            handle_,
                            address,
                            buf,
                            n_requests,
                            address_spec<A>::MBLT,
                            &n_written)
                };
                HANDLE_CAEN_ERROR(err, "MBLTWriteCycle failed");
                return static_cast<unsigned>(n_written);
            }
    }
}

#endif
