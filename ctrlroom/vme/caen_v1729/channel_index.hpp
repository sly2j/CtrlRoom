#ifndef CTRLROOM_VME_CAEN_V1729A_CHANNEL_INDEX
#define CTRLROOM_VME_CAEN_V1729A_CHANNEL_INDEX

#include <ctrlroom/vme/caen_v1729/spec.hpp>
#include <ctrlroom/vme/vme64.hpp>
#include <cstddef>

// helper routine to select the correct channel indexing
// routine depending on the addressing mode
//    - A24-D16: shorts 3 -> 0
//    - A32-D32: ints 3->2 and 2->1
//    NOTE: due to reinterpreting the array of 32-bit integers
//          as 16-bit shorts on a LITTLE-ENDIAN system, this
//          gives the final in-memory order of
//          shorts 2->3->1->2. This routine takes care of this
//          step.

namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {
            template <addressing_mode A>
                struct channel_index;
            template <>
                struct channel_index<addressing_mode::A24> {
                    static size_t calc(const size_t chan) {
                        return properties::N_CHANNELS - (chan + 1);
                    }
                };
            template <>
                struct channel_index<addressing_mode::A32> {
                    static size_t calc(const size_t chan) {
                        return (chan + properties::N_CHANNELS / 2) 
                            % properties::N_CHANNELS;
                    }
                };
        }
    }
}

#endif
