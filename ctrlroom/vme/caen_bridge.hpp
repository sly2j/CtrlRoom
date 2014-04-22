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
                    void wait_for_irq(size_t timeout) const;

                protected:
                    // Single
                    template <addressing_mode A, transfer_mode D>
                        size_t read_single(
                                const typename address_spec<A>::ptr_type address,
                                typename transfer_spec<D>::ptr_type val) const;
                    template <addressing_mode A, transfer_mode D>
                        size_t write_single(
                                const typename address_spec<A>::ptr_type address,
                                typename transfer_spec<D>::ptr_type val) const;
                    // BLT
                    template <addressing_mode A, transfer_mode D>
                        size_t read_blt(
                                const typename address_spec<A>::ptr_type address,
                                typename transfer_spec<D>::ptr_type buf,
                                size_t n_requests) const;
                    template <addressing_mode A, transfer_mode D>
                        size_t write_blt(
                                const typename address_spec<A>::ptr_type address,
                                typename transfer_spec<D>::ptr_type buf,
                                size_t n_requests) const;
                    // MBLT
                    template <addressing_mode A>
                        size_t read_mblt(
                                const typename address_spec<A>::ptr_type address,
                                transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                                size_t n_requests) const;
                    template <addressing_mode A>
                        size_t write_mblt(
                                const typename address_spec<A>::ptr_type address,
                                transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                                size_t n_requests) const;
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

                    VME_FRIEND_MASTER(base_type);
        };
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// implementation: caen_bridge
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {

        inline
        void caen_bridge::wait_for_irq() const {
            wait_for_irq(timeout_);
        }

        template <addressing_mode A, transfer_mode D>
            size_t caen_bridge::read_single(
                    const typename address_spec<A>::ptr_type address,
                    typename transfer_spec<D>::ptr_type val) const {

                constexpr CVAddressModifier am {
                    static_cast<CVAddressModifier>(address_spec<A>::DATA)
                };
                constexpr CVDataWidth width {
                    static_cast<CVDataWidth>(transfer_spec<D>::WIDTH)
                };
                CVErrorCodes err {
                    CAENVME_ReadCycle(handle_, address, val, am, width)
                };
                HANDLE_CAEN_ERROR(err, "ReadCycle call failed");
                return {1};
            }
        template <addressing_mode A, transfer_mode D>
            size_t caen_bridge::write_single(
                    const typename address_spec<A>::ptr_type address,
                    typename transfer_spec<D>::ptr_type val) const {

                constexpr CVAddressModifier am {
                    static_cast<CVAddressModifier>(address_spec<A>::DATA)
                };
                constexpr CVDataWidth width {
                    static_cast<CVDataWidth>(transfer_spec<D>::WIDTH)
                };
                CVErrorCodes err {
                    CAENVME_WriteCycle(handle_, address, val, am, width)
                };
                HANDLE_CAEN_ERROR(err, "WriteCycle call failed");
                return {1};
            }

        template <addressing_mode A, transfer_mode D>
            size_t caen_bridge::read_blt(
                    const typename address_spec<A>::ptr_type address,
                    typename transfer_spec<D>::ptr_type buf,
                    size_t n_requests) const {
                int n_read {0};
                constexpr CVAddressModifier am {
                    static_cast<CVAddressModifier>(address_spec<A>::BLT)
                };
                constexpr CVDataWidth width {
                    static_cast<CVDataWidth>(transfer_spec<D>::WIDTH)
                };
                // requests in bytes
                n_requests *= transfer_spec<D>::WIDTH;
                CVErrorCodes err {
                    CAENVME_BLTReadCycle(
                            handle_,
                            address,
                            buf,
                            n_requests,
                            am,
                            width,
                            &n_read)
                };
                HANDLE_CAEN_ERROR(err, "BLTReadCycle failed");
                // number of read values in D words
                n_read /= transfer_spec<D>::WIDTH;
                return static_cast<size_t>(n_read);
            }
        template <addressing_mode A, transfer_mode D>
            size_t caen_bridge::write_blt(
                    const typename address_spec<A>::ptr_type address,
                    typename transfer_spec<D>::ptr_type buf,
                    size_t n_requests) const {
                int n_written {0};
                constexpr CVAddressModifier am {
                    static_cast<CVAddressModifier>(address_spec<A>::BLT)
                };
                constexpr CVDataWidth width {
                    static_cast<CVDataWidth>(transfer_spec<D>::WIDTH)
                };
                // requests in bytes
                n_requests *= transfer_spec<D>::WIDTH;
                CVErrorCodes err {
                    CAENVME_BLTWriteCycle(
                            handle_,
                            address,
                            buf,
                            n_requests,
                            am,
                            width,
                            &n_written)
                };
                HANDLE_CAEN_ERROR(err, "BLTWriteCycle failed");
                // number of written values in D words
                n_written /= transfer_spec<D>::WIDTH;
                return static_cast<size_t>(n_written);
            }
        template <addressing_mode A>
            size_t caen_bridge::read_mblt(
                    const typename address_spec<A>::ptr_type address,
                    transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                    size_t n_requests) const {
                int n_read {0};
                constexpr CVAddressModifier am {
                    static_cast<CVAddressModifier>(address_spec<A>::MBLT)
                };
                // requests in bytes
                n_requests *= transfer_spec<transfer_mode::MBLT>::WIDTH;
                CVErrorCodes err {
                    CAENVME_MBLTReadCycle(
                            handle_,
                            address,
                            buf,
                            n_requests,
                            am,
                            &n_read)
                };
                //DBG
                //DBGstatic unsigned DBG {0};
                //DBGfor (unsigned i {0}; i < n_read/8; ++i) {
                //DBG    std::cout << "MBLT " << DBG << ":  " << i << "  "<< buf[i] << std::endl;
                //DBG}
                //DBG++DBG;
                //DBG
                HANDLE_CAEN_ERROR(err, "MBLTReadCycle failed");
                // number of read values in 64-bit words
                n_read /= transfer_spec<transfer_mode::MBLT>::WIDTH;
                return static_cast<size_t>(n_read);
            }
        template <addressing_mode A>
            size_t caen_bridge::write_mblt(
                    const typename address_spec<A>::ptr_type address,
                    transfer_spec<transfer_mode::MBLT>::ptr_type buf,
                    size_t n_requests) const {
                int n_written {0};
                constexpr CVAddressModifier am {
                    static_cast<CVAddressModifier>(address_spec<A>::MBLT)
                };
                // requests in bytes
                n_requests *= transfer_spec<transfer_mode::MBLT>::WIDTH;
                CVErrorCodes err {
                    CAENVME_MBLTWriteCycle(
                            handle_,
                            address,
                            buf,
                            n_requests,
                            am,
                            &n_written)
                };
                HANDLE_CAEN_ERROR(err, "MBLTWriteCycle failed");
                // number of written values in 64-bit words
                n_written /= transfer_spec<transfer_mode::MBLT>::WIDTH;
                return static_cast<size_t>(n_written);
            }
    }
}

#endif
