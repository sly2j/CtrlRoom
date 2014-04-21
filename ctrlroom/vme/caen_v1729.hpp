#ifndef CTRLROOM_VME_CAEN_V1729A_LOADED
#define CTRLROOM_VME_CAEN_V1729A_LOADED

#include <ctrlroom/vme/caen_v1729/spec.hpp>
#include <ctrlroom/vme/slave.hpp>

#include <ctrlroom/util/assert.hpp>
#include <ctrlroom/util/exception.hpp>
#include <ctrlroom/util/logger.hpp>
#include <ctrlroom/util/mixin.hpp>

#include <algorithm>
#include <array>
#include <limits>
#include <string>
#include <memory>

namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {

            // calibration POD struct
            template <class Board> 
                struct calibration {
                    using board_type = Board;
                    using memory_type = typename board_type::memory_type;
                    using vernier_type = typename board_type::vernier_type;

                    // The posttrig argument is optional. 
                    // The user should not touch this variable, as the V1729 board
                    // class will overwrite that variable with the correct value
                    calibration(
                            const memory_type& ped, 
                            const vernier_type& min, 
                            const vernier_type& max,
                            const unsigned posttrig = 0);

                    memory_type pedestal;
                    vernier_type vernier_min;
                    vernier_type vernier_max;
                    unsigned post_trig;
                };

            // an array-like "channel view" interface to a V1729 buffer,
            // as well as a related iterator
            template <class Board>
                struct channel_view;
            template <class View>
                struct channel_view_iterator;

            // buffer to store measured ADC data, and transparently provide
            // intuitive access to the underlying circular buffer
            template <class Board>
                class buffer {
                    public:
                        using board_type = Board;
                        using calibration_type = typename board_type::calibration_type;
                        using memory_type = typename board_type::memory_type;
                        using value_type = typename board_type::value_type;
                        using view_type = channel_view<buffer>;

                        // return value at index <idx> for channel <chan> from the buffer,
                        // taking care of the circular buffer unfolding.
                        // Requires a valid calibration to be loaded! (will _segfault_ if
                        // not the case!)
                        value_type get(const unsigned chan, unsigned idx) const;

                        // get the integrated ADC response
                        // (the range-less version intergrates between min and max
                        value_type integrate(
                                const unsigned chan, 
                                const std::pair<unsigned, unsigned>& range) const;
                        value_type integrate (const unsigned chan) const;

                        constexpr unsigned size() const;
                        
                        // get a channel view interface to a channel
                        // for more elegant array-like access
                        view_type channel(const unsigned chan) const;

                    private:
                        value_type mask(const value_type val) const;
                        
                        // do the index magic to address the circular buffer
                        // returns the internal buffer address for this index
                        // and channel number
                        unsigned fold_index(
                                const unsigned chan, 
                                unsigned idx) const;

                        // calibrate the buffer, called by the V1729 board class
                        // after the buffer is filled with new data
                        void calibrate(
                                std::shared_ptr<const calibration_type>& cal,
                                const unsigned trig_rec);

                        // helper function for calibrate() to get the correct vernier offset
                        unsigned vernier();

                        memory_type buffer_;
                        unsigned buffer_end_;
                        std::shared_ptr<const calibration_type> calibration_;

                        friend board_type; 
                };

            // Generic implementation of the V1729 and v1729a board
            // specific typedefs for either boards are provided below
            // (in the main ctrlroom::vme namespace)
            // CONFIGURATION FILE OPTIONS:
            //      * Trigger type: <id>.triggerType (internal, external, ...)
            //      * Trigger settings: <id>.triggerSettings ([rising, ...])
            //      * Trigger threshold: <id>.triggerThreshold (-1000mV...1000mV)
            //      * PRETRIG: <id>.pretrig (MIN, ... 0xFFFF)
            //      * POSTTRIG: <id>.postTrig (7, ... 0xFFFF)
            //      * sampling freq.: <id>.samplingFrequency (2GHz, 1GHz)
            //  optional EXPERT USAGE ONLY (will cause calibrations to hang!!!)
            //  (default to true)
            //      * IRQ: <id>.enableIRQ (true, false)
            //      * auto restart acquisition: <id>.autoRestartAcq (true, false)
            //  optional, default to [ALL]
            //      * Trigger channel source: <id>.triggerChannelSource ([CH0,CH1,CH2,CH3,ALL])
            //      * Channel mask: <id>.channelMask ([CH0, CH1, CH2, CH3, ALL])
            //  optional, defualt to single
            //      * multiplexing mode: <id>.channelMultiplexing (single, duplex, quadruplex)
            template <class Master, 
                            submodel M,
                            addressing_mode A, 
                            transfer_mode DBLT = transfer_mode::MBLT>
                class board
                    : public slave<Master, A, transfer_mode::D16, DBLT>
                    , public properties
                    , public extra_properties<M> {
                        public: 
                            static constexpr const char* TRIGGER_TYPE_KEY {"triggerType"};
                            static constexpr const char* TRIGGER_SETTINGS_KEY {"triggerSettings"};
                            static constexpr const char* TRIGGER_THRESHOLD_KEY {"triggerThreshold"};
                            static constexpr const char* PRETRIG_KEY {"preTrig"};
                            static constexpr const char* POSTTRIG_KEY {"postTrig"};
                            static constexpr const char* SAMPLING_FREQUENCY_KEY {"samplingFrequency"};
                            // optional (default to true)
                            static constexpr const char* ENABLE_IRQ_KEY {"enableIRQ"};
                            static constexpr const char* AUTO_RESTART_ACQ_KEY {"autoRestartAcq"};
                            // optional (default to all)
                            static constexpr const char* TRIGGER_CHANNEL_SOURCE_KEY {"triggerChannelSource"};
                            static constexpr const char* CHANNEL_MASK_KEY {"channelMask"};
                            // optional (defaults to single)
                            static constexpr const char* CHANNEL_MULTIPLEXING_KEY {"channelMultiplexing"};

                            using base_type = slave<Master, A, transfer_mode::D16, DBLT>;
                            using master_type = Master;
                            using instructions = caen_v1729_impl::instructions<A>;
                            using buffer_type = buffer<board>;
                            using calibration_type = calibration<board>;
                            using single_data_type = typename base_type::single_data_type;
                            using blt_data_type = typename base_type::blt_data_type;
                            using address_type = typename base_type::address_type;

                            board(  const std::string& identifier,
                                    const ptree& settings,
                                    std::shared_ptr<master_type>& master,
                                    const calibration_type& cal);

                            // read the measured pulse from memory
                            // will automatically restart acquisition
                            // if autoRestartAcq is set to true
                            unsigned read_pulse(buffer_type& buf) const;

                            // calibrate the verniers
                            static std::pair<vernier_type, vernier_type> calibrate_verniers(
                                    const std::string& identifier,
                                    const ptree& settings,
                                    std::shared_ptr<master_type>& master);

                            // do <n_acquisitions> random measurements to determine
                            // the board pedestal values
                            // note that the first <MEMORY_HEADER_SIZE> values are irrelevant, 
                            // as they correspond to the memory header
                            static memory_type measure_pedestal(
                                    const std::string& identifier,
                                    const ptree& settings,
                                    std::shared_ptr<master_type>& master,
                                    unsigned n_acquisitions=50);

                        private:
                            // static members so the calibration functions can also use
                            // the general initialization routines
                            static void init(const base_type& b);
                            static void init_trigger(const base_type& b);
                            static void init_mode_register(const base_type& b);
                            static void init_digitizer(const base_type& b);
                            static void init_window(const base_type& b);

                            std::shared_ptr<const calibration_type> calibration_; 
                    };
        }
        // actual V1729a and V1729 aliases
        template <class Master, 
                    addressing_mode A, 
                    transfer_mode DBLT = transfer_mode::MBLT>
            using caen_v1729a = caen_v1729_impl::board<
                                    Master, 
                                    caen_v1729_impl::submodel::V1729A,
                                    A,
                                    DBLT>;
        template <class Master, 
                    addressing_mode A, 
                    transfer_mode DBLT = transfer_mode::MBLT>
            using caen_v1729 = caen_v1729_impl::board<
                                    Master, 
                                    caen_v1729_impl::submodel::V1729,
                                    A,
                                    DBLT>;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// definition: channel_view
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {

            // STL-vector-like interface to a single-channel
            // view in a V1729 buffer
            template <class Board>
                class channel_view {
                    public:
                        using board_type = Board;
                        using buffer_type = buffer<board_type>;
                        using value_type = typename board_type::value_type;
                        using iterator = channel_view_iterator<channel_view>;

                        constexpr channel_view(
                                const unsigned channel, 
                                const buffer_type& buffer);

                        value_type operator[](const unsigned idx) const;
                        value_type at(const unsigned idx) const;
                        constexpr unsigned size() const;
                        iterator begin() const;
                        iterator end() const;
                        unsigned channel_number() const;

                    private:
                        const unsigned channel_;
                        const buffer_type& buffer_;
                };

        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// definition: channel_view_iterator
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {
            template <class View>
                class channel_view_iterator
                    : public std::iterator<std::random_access_iterator_tag,
                                           typename View::value_type>
                    , public comparison_mixin<channel_view_iterator<View>>
                    , public postfix_mixin<channel_view_iterator<View>>
                    , public add_subtract_mixin<channel_view_iterator<View>> {
                        public:
                            using view_type = View;
                            using base_type = std::iterator<
                                std::random_access_iterator_tag,
                                typename View::value_type>;
                            using value_type = typename base_type::value_type;
                            using difference_type = typename base_type::difference_type;

                            // constructors
                            constexpr channel_view_iterator();
                            constexpr channel_view_iterator(const view_type& view);
                            
                            // dereference operator
                            value_type operator*() const;
                            // operator-> doesn't make sense, as lvalues
                            // aren't available due to masking

                            // comparison operators (expanded in comparison_mixin)
                            bool operator==(const channel_view_iterator& rhs) const;
                            bool operator<(const channel_view_iterator& rhs) const;
                            
                            // arithmetic operators, expanded in add_subtract_mixin
                            channel_view_iterator& operator+=(difference_type n);

                            // support for difference between two iterators
                            difference_type operator- (
                                    const channel_view_iterator& it) const;

                            // prefix operators, complemented with postfix versions by
                            // postfix_mixin
                            channel_view_iterator& operator++();
                            channel_view_iterator& operator--();
                            
                            // offset dereference operator (C++11)
                            value_type operator[] (unsigned i) const;
                            // swap() and N + <iterator> not implemented
                            
                        private:
                            void set_end();

                            const view_type* view_;
                            unsigned idx_;
                            unsigned end_idx_;

                            friend view_type;
                    };
        }
    }
}



//////////////////////////////////////////////////////////////////////////////////////////
// implementation: V1729
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {

            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                board<Master, M, A, DBLT>::board(
                        const std::string& identifier,
                        const ptree& settings,
                        std::shared_ptr<Master>& master,
                        const board<Master, M, A, DBLT>::calibration_type& cal)
                    : base_type{identifier, settings, master} {
                        init(*this);
                        calibration_.reset(
                                new calibration_type {
                                    cal.pedestal,
                                    cal.vernier_min,
                                    cal.vernier_max,
                                    this->conf_.template get<uint16_t>(POSTTRIG_KEY)
                                }
                        );
                    }

            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                unsigned board<Master, M, A, DBLT>::read_pulse(
                        board<Master, M, A, DBLT>::buffer_type& buf) const {
                    unsigned nread {
                        read(instructions::RAM_DATA, buf.buffer_)
                    };
                    typename memory_type::value_type trig_rec;
                    // read the trig_rec and automatically restart
                    // acquisition
                    read(instructions::RAM_DATA, trig_rec);
                    buf.calibrate(calibration_, trig_rec);
                    return nread;
                }

            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                auto board<Master, M, A, DBLT>::calibrate_verniers(
                        const std::string& identifier,
                        const ptree& settings,
                        std::shared_ptr<Master>& master) 
                -> std::pair<vernier_type, vernier_type> {
                    LOG_INFO(identifier, "Calibrating the verniers");
                    // initialize the arrays
                    vernier_type min, max;
                    std::fill(min.begin(), min.end(), 
                            std::numeric_limits<typename vernier_type::value_type>::max());
                    std::fill(max.begin(), max.end(), 
                            std::numeric_limits<typename vernier_type::value_type>::min());
                    // board handle
                    base_type b {identifier, settings, master};
                    init(b);
                    // random trigger for all channels
                    b.write(instructions::TRIGGER_TYPE, 
                            trigger_type::SOFTWARE | trigger_settings::RANDOM);
                    b.write(instructions::CHANNEL_MASK, channel::CALL);
                    // read zero columns from memory for fast calibration
                    b.write(instructions::NB_OF_COLS_TO_READ, 0);
                    // (heap) array to store the vernier data
                    using vernier_memory_type = 
                        std::array<memory_type::value_type, VERNIER_MEMORY_SIZE>;
                    std::unique_ptr<vernier_memory_type> vbuf {new vernier_memory_type};
                    // acquisition loop
                    b.write(instructions::START_ACQUISITION, 1);
                    unsigned nread {b.read(instructions::RAM_DATA, vbuf)};
                    tassert(nread == VERNIER_MEMORY_SIZE,
                            "Problem reading the vernier calibration data");
                    // process the data
                    for (unsigned i {0}; i < vbuf->size(); ++i) {
                        unsigned channel {N_CHANNELS - (i % N_CHANNELS) - 1};
                        if ((*vbuf)[i] < min[channel]) {
                            min[channel] = (*vbuf)[i];
                        }
                        if ((*vbuf)[i] > max[channel]) {
                            max[channel] = (*vbuf)[i];
                        }
                    }
                    return {min, max};
                }

            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                auto board<Master, M, A, DBLT>::measure_pedestal(
                        const std::string& identifier,
                        const ptree& settings,
                        std::shared_ptr<Master>& master,
                        unsigned n_acquisitions) 
                -> memory_type {
                    LOG_INFO(identifier, "Measuring the board pedestal");
                    // init the arrays
                    memory_type ped {0};
                    std::array<double, MEMORY_SIZE + MEMORY_HEADER_SIZE> sum {0.};
                    // temporary board handle
                    base_type b {identifier, settings, master};
                    init(b);
                    b.write(instructions::RESET, 1);
                    // random trigger for all channels and cells
                    b.write(instructions::TRIGGER_TYPE, 
                            trigger_type::SOFTWARE | trigger_settings::RANDOM);
                    b.write(instructions::CHANNEL_MASK, channel::CALL);
                    b.write(instructions::NB_OF_COLS_TO_READ, N_CELLS);
                    // do <n_acquisitions> acquisitions
                    for (unsigned i {0}; i < n_acquisitions; ++i) {
                        b.write(instructions::START_ACQUISITION, 1);
                        master->wait_for_irq();
                        unsigned nread {
                            b.template read<short unsigned int, MEMORY_SIZE>(instructions::RAM_DATA, ped)
                        };
                        tassert(nread == MEMORY_SIZE,
                                "Problem measuring the pedestal");
                        std::transform(
                                sum.begin(), sum.end(), 
                                ped.begin(), 
                                sum.begin(), 
                                [=](double a, memory_type::value_type b) {
                                    return a 
                                        + static_cast<double>(b & extra_properties<M>::MEMORY_MASK) 
                                            / n_acquisitions;
                                }
                        );
                    }
                    std::copy(sum.begin(), sum.end(), ped.begin());
                    return ped;
                }

            // the init functions are implemented as static member functions to play
            // nice with the calibration methods. This however complicates things
            // because they don't have access to a this pointer. The board access is
            // circumvented by creating a raw temporary VME slave object, and using this
            // directly. This has the unfortunate side effect of a somewhat cumbersome
            // syntax (see all the "template" keywords specifiers for member functions)
            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                void board<Master, M, A, DBLT>::init(
                        const board<Master, M, A, DBLT>::base_type& b) {
                    b.write(instructions::RESET, 0x1);
                    init_trigger(b);
                    init_mode_register(b);
                    init_digitizer(b);
                    init_window(b);
                }
            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                void board<Master, M, A, DBLT>::init_trigger(
                        const board<Master, M, A, DBLT>::base_type& b) {
                    // enable the trigger rate monitor
                    b.write(instructions::RATE_REG, 0x1);

                    // set the trigger type
                    trigger_type type {
                        b.conf_.template get<trigger_type>(TRIGGER_TYPE_KEY, TRIGGER_TYPE_TRANSLATOR)
                    };
                    single_data_type bitpattern {
                        b.conf_.get_bitpattern(
                                TRIGGER_SETTINGS_KEY,
                                TRIGGER_SETTINGS_TRANSLATOR)
                    };
                    bitpattern |= type;
                    b.write(instructions::TRIGGER_TYPE, bitpattern);

                    // set the channel source in case of "internal" and "or" trigger
                    if (type == trigger_type::INTERNAL
                            || type == trigger_type::OR) {
                        single_data_type channel_pattern {
                            b.conf_.get_bitpattern(
                                    TRIGGER_CHANNEL_SOURCE_KEY,
                                    channel::CALL,
                                    CHANNEL_TRANSLATOR)
                        };
                        b.write(instructions::TRIGGER_CHANNEL_SOURCE, 
                                channel_pattern);
                    }

                    // set the trigger threshold as long as 
                    //   * the trigger is not external
                    //   _AND_
                    //   * trigger_setting::DIRECT_EXTERNAL isn't set
                    if (!(type == trigger_type::EXTERNAL
                                && (bitpattern & trigger_settings::DIRECT_EXTERNAL))) {
                        float f_threshold {
                            b.conf_.template get<float>(TRIGGER_THRESHOLD_KEY)
                        };
                        if (fabs(f_threshold) > MAX_ABS_TRIGGER_THRESHOLD) {
                            throw b.conf_.value_error(
                                    TRIGGER_THRESHOLD_KEY,
                                    std::to_string(f_threshold));
                        }
                        // convert floating point value to a 12-bit integer
                        // (-1V --> 1V becomes 0x000 --> 0xFFF)
                        single_data_type i_threshold {
                            static_cast<single_data_type>(
                                    (f_threshold + MAX_ABS_TRIGGER_THRESHOLD)
                                        /(2.*MAX_ABS_TRIGGER_THRESHOLD) * 0xFFF)
                        };
                        b.write(instructions::TRIGGER_THRESHOLD_DAC, i_threshold);
                        b.write(instructions::LOAD_TRIGGER_THRESHOLD_DAC, 0x1);
                    }
                }
            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                void board<Master, M, A, DBLT>::init_mode_register(
                        const board<Master, M, A, DBLT>::base_type& b) {
                    // bit 1: 12/14bit mode
                    single_data_type mode_register = extra_properties<M>::BIT_MODE;
                    // bit 0: enable/disable VME IRQ from this board
                    //        defaults to 0x1
                    mode_register |= 
                        b.conf_.template get<single_data_type>(
                                ENABLE_IRQ_KEY, 
                                0x1,
                                BINARY_TRANSLATOR);
                    // bit 2: enable auto restart acquisition on read
                    //        of TRIG_REC, defaults to 1 (0x4)
                    mode_register |= 
                        (b.conf_.get(
                                AUTO_RESTART_ACQ_KEY,
                                0x1,
                                BINARY_TRANSLATOR)) << 2;
                    b.write(instructions::MODE_REGISTER, mode_register);
                }
            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                void board<Master, M, A, DBLT>::init_digitizer(
                        const board<Master, M, A, DBLT>::base_type& b) {
                    // sampling frequency
                    single_data_type clock {
                        b.conf_.get(
                                SAMPLING_FREQUENCY_KEY,
                                SAMPLING_FREQUENCY_TRANSLATOR)
                    };
                    b.write(instructions::FP_FREQUENCY, clock);
                    // number of cols to read (all)
                    b.write(instructions::NB_OF_COLS_TO_READ,
                            N_CELLS);
                    // channels to read (default to all)
                    channel channel_pattern {
                        b.conf_.get_bitpattern(
                                CHANNEL_MASK_KEY,
                                channel::CALL,
                                CHANNEL_TRANSLATOR)
                    };
                    b.write(instructions::CHANNEL_MASK, 
                            channel_pattern);
                    // number of channels for multiplexing 
                    // (1 channel per channel)
                    single_data_type n_channels {
                        b.conf_.get(
                                CHANNEL_MULTIPLEXING_KEY,
                                channel_multiplexing::C_SINGLE,
                                CHANNEL_MULTIPLEXING_TRANSLATOR)
                    };
                    b.write(instructions::NUMBER_OF_CHANNELS,
                            n_channels);
                }
                // initialize the pre- and post-trig windows
            template <class Master, submodel M, addressing_mode A, transfer_mode DBLT>
                void board<Master, M, A, DBLT>::init_window(
                        const board<Master, M, A, DBLT>::base_type& b) {
                    // pretrig
                    uint16_t pretrig {b.conf_.template get<uint16_t>(PRETRIG_KEY)};
                    // sampling frequency for value checking
                    single_data_type fsample {
                        b.conf_.get(
                                SAMPLING_FREQUENCY_KEY,
                                SAMPLING_FREQUENCY_TRANSLATOR)
                    };
                    if ((fsample == sampling_frequency::FS_2GHZ
                            && pretrig < MIN_PRETRIG_2GHZ)
                        || (fsample == sampling_frequency::FS_1GHZ
                            && pretrig < MIN_PRETRIG_1GHZ)) {
                        throw b.conf_.value_error(
                                PRETRIG_KEY, 
                                std::to_string(pretrig));
                    }
                    // postrig values
                    uint16_t posttrig {b.conf_.template get<uint16_t>(POSTTRIG_KEY)};
                    // validate posttrig
                    if (posttrig < INTRINSIC_POSTTRIG) {
                        throw b.conf_.value_error(
                                POSTTRIG_KEY, 
                                std::to_string(posttrig));
                    }
                    posttrig -= INTRINSIC_POSTTRIG;
                    // write to registers
                    b.write(instructions::PRETRIG.LSB, pretrig & 0xFF);
                    b.write(instructions::PRETRIG.MSB, (pretrig >> 2) & 0xFF);
                    b.write(instructions::POSTTRIG.LSB, posttrig & 0xFF);
                    b.write(instructions::POSTTRIG.MSB, (posttrig >> 2) & 0xFF);
                }
                    
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// implementation: calibration
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {

            template <class Board>
                calibration<Board>::calibration(
                        const memory_type& ped, 
                        const vernier_type& min, 
                        const vernier_type& max,
                        const unsigned post)
                        : pedestal {ped}
                        , vernier_min {min}
                        , vernier_max {max} 
                        , post_trig {post} {}

        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// implementation: buffer
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {

            template <class Board>
                auto buffer<Board>::get (
                        const unsigned chan, unsigned idx) const
                -> value_type {
                    idx = fold_index(chan, idx);
                    return mask(buffer_[idx]) - calibration_->pedestal[idx];
                }

            template <class Board>
                auto buffer<Board>::integrate (
                        const unsigned chan, 
                        const std::pair<unsigned, unsigned>& range) const
                -> value_type {
                    value_type sum {0};
                    for (unsigned i {range.first}; i < range.second; ++i) {
                        sum += get(chan, i);
                    }
                    return sum;
                }
            template <class Board>
                auto buffer<Board>::integrate (
                        const unsigned chan) const 
                -> value_type {
                    return integrate(chan, {0, size()});
                }

            template <class Board>
                constexpr unsigned buffer<Board>::size() const {
                    return (board_type::MEMORY_DATA_SIZE
                                - board_type::MEMORY_DATA_SKIP)
                            / board_type::N_CHANNELS;
                }

            template <class Board>
                auto buffer<Board>::channel(
                        const unsigned chan) const 
                -> view_type {
                    return {chan, *this};
                }

            template <class Board>
                auto buffer<Board>::mask(
                        const buffer<Board>::value_type val) const 
                -> value_type {
                    return val & board_type::MEMORY_MASK;
                }
            template <class Board>
                unsigned buffer<Board>::fold_index(
                        const unsigned chan, 
                        unsigned idx) const {
                    // The first values in the buffer cannot be trusted
                    idx += board_type::MEMORY_DATA_SKIP;
                    // fold the index into the circular buffer
                    idx = (buffer_end_ + idx + 1) % board_type::MEMORY_DATA_SIZE;
                    // channels are stored backwards 3->0
                    idx = (idx + 1) * board_type::N_CHANNELS - (chan + 1);
                    // data is offset by the header
                    idx += board_type::MEMORY_HEADER_SIZE;
                    // that's all
                    return idx;
                }

            template <class Board>
                void buffer<Board>::calibrate(
                        std::shared_ptr<const buffer<Board>::calibration_type>& cal,
                        const unsigned trig_rec) {
                    tassert(cal, "null pointer error");
                    calibration_ = cal;
                    buffer_end_ = board_type::N_CELLS 
                                        - (trig_rec - cal.post_trig);
                    buffer_end_ *= board_type::LINES_PER_CELL;
                    buffer_end_ -= vernier();
                }

            template <class Board>
                unsigned buffer<Board>::vernier() {
                    double v {0};
                    for (unsigned i {0}; 
                            i < board_type::N_CHANNELS; ++i) {
                        // Vernier is stored as words 4-7 in the buffer
                        v += double{buffer_[board_type::MEMORY_VERNIER_INDEX + i] 
                                        - calibration_->vernier_min[i]}
                            / double{calibration_->vernier_max[i] - 
                                        calibration_->vernier_min[i]};
                    };

                    return static_cast<unsigned>(
                            board_type::LINES_PER_CELL * v / board_type::N_CHANNELS);
                }


        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// implementation: channel_view
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {
            
            template <class Board>
                constexpr channel_view<Board>::channel_view(
                        const unsigned channel, 
                        const channel_view<Board>::buffer_type& buffer)
                    : channel_ {channel}
                    , buffer_ {buffer} {}

            template <class Board>
                auto channel_view<Board>::operator[](
                        const unsigned idx) const 
                -> value_type {
                    return buffer_.get(channel_, idx);
                }
            template <class Board>
                auto channel_view<Board>::at(
                        const unsigned idx) const 
                -> value_type {
                    if (idx > size()) {
                        throw exception("out_of_range", 
                                        "Invalid index requested from channel_view");
                    }
                    return (*this)[idx];
                }
            template <class Board>
                constexpr unsigned channel_view<Board>::size() const {
                    return buffer_.size();
                }
            template <class Board>
                auto channel_view<Board>::begin() const 
                -> iterator {
                    return {this};
                }
            template <class Board>
                auto channel_view<Board>::end() const 
                -> iterator {
                    return iterator{this}.set_end();
                }
            template <class Board>
                unsigned 
                channel_view<Board>::channel_number() const {
                    return channel_;
                }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// implementation: channel_view_iterator
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom {
    namespace vme {
        namespace caen_v1729_impl {
            template <class View>
                constexpr channel_view_iterator<View>::channel_view_iterator() 
                    : view_ {nullptr}
                    , idx_ {0}
                    , end_idx_ {0} {}

            template <class View>
                constexpr channel_view_iterator<View>::channel_view_iterator(
                        const view_type& view)
                    : view_ {&view}
                    , idx_ {0} 
                    , end_idx_ {view.size()} {}

            template <class View>
                auto channel_view_iterator<View>::operator*() const 
                -> value_type {
                    return (*view_)[idx_];
                }
            template <class View>
                bool channel_view_iterator<View>::operator==(
                        const channel_view_iterator<View>& rhs) const {
                    return (view_ == rhs.view_ && idx_ == rhs.idx_);
                }
            template <class View>
                bool channel_view_iterator<View>::operator<(
                        const channel_view_iterator<View>& rhs) const {
                    return (view_ == rhs.view_ && idx_ < rhs.idx_);
                }
            template <class View>
                auto channel_view_iterator<View>::operator+=(
                        channel_view_iterator<View>::difference_type n) 
                -> channel_view_iterator& {
                    if (idx_ != end_idx_) {
                        // overflow
                        if (n > 0 && idx_ + n > end_idx_) {
                            set_end();
                        // undeflow
                        } else if (n < 0 && idx_ < n) {
                            set_end();
                        // default
                        } else {
                            idx_ += n;
                        }
                    }
                    return *this;
                }
            template <class View>
                auto channel_view_iterator<View>::operator- (
                        const channel_view_iterator<View>& b) const 
                -> difference_type {
                    return static_cast<difference_type>(idx_)
                                - static_cast<difference_type>(b.idx_);
                }
            template <class View>
                auto channel_view_iterator<View>::operator++() 
                -> channel_view_iterator& {
                    if (idx_ < end_idx_) {
                        ++idx_;
                    }
                    return *this;
                }
            template <class View>
                auto channel_view_iterator<View>::operator--()
                -> channel_view_iterator& {
                    if (idx_ > 0) {
                        ++idx_;
                    } else {
                        set_end();
                    }
                    return *this;
                }
            template <class View>
                auto channel_view_iterator<View>::operator[] (unsigned i) const 
                -> value_type {
                    return (*view_)[idx_ + i];
                }
            template <class View>
                void channel_view_iterator<View>::set_end() {
                    idx_ = 0;
                    end_idx_ = 0;
                    view_ = nullptr;
                }
        }
    }
}
                

namespace ctrlroom {
    namespace vme {

#if 0
        template <class Master,
                    addressing_mode A = addressing_mode::A32,
                    transfer_mode D = transfer_mode::MBLT>
            using caen_v1729 = 
                        caen_v1729_impl::board<caen_v1729_impl::submodel::V1729>;
        template <class Master,
                    addressing_mode A = addressing_mode::A32,
                    transfer_mode D = transfer_mode::MBLT>
            using caen_v1729a = 
                        caen_v1729_impl::board<caen_v1729_impl::submodel::V1729A>;

#endif
    }
}



#endif
