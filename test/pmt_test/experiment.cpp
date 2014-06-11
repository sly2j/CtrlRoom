#include <ctrlroom/vme/caen_bridge.hpp>
#include <ctrlroom/vme/caen_v1729.hpp>
#include <ctrlroom/vme/vme64.hpp>

#include <ctrlroom/util/stringify.hpp>
#include <ctrlroom/util/configuration.hpp>
#include <ctrlroom/util/exception.hpp>
#include <ctrlroom/util/assert.hpp>
#include <ctrlroom/util/logger.hpp>
#include <ctrlroom/util/io.hpp>
#include <ctrlroom/util/root.hpp>

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <exception>

#include <TFile.h>
#include <TTree.h>
#include <TStyle.h>

#include <TCanvas.h>
#include <TH1I.h>

#include <boost/filesystem.hpp>

using namespace ctrlroom;

using bridge_type = vme::caen_bridge;
using adc_type = vme::caen_v1729a<bridge_type, vme::addressing_mode::A32,
                                  vme::transfer_mode::D32,   // single
                                  vme::transfer_mode::MBLT>; // BLT

const std::string CALIBRATION_CMD{"calibrate"};

struct create_path_proxy {
  create_path_proxy(const configuration& conf, const std::string& path_key)
      : path{conf.get<std::string>(path_key)} {
    if (boost::filesystem::exists(path)) {
      throw conf.value_error(path_key, path);
    }
    boost::filesystem::create_directories(path);
  }
  const std::string path;
};

class experiment {
public:
  // config file keys
  static constexpr const char* EXPERIMENT_KEY{"experiment"};
  static constexpr const char* NAME_KEY{"name"};

  static constexpr const char* CALIBRATION_KEY{"path.calibration"};
  static constexpr const char* OUTDIR_KEY{"path.output"};
    
  static constexpr const char* SIGNAL_PATH{"signal."};
  static constexpr const char* INTEGRAL_PATH{"integral."};
  static constexpr const char* PMT_PATH{"PMT."};

  static constexpr const char* N_PULSES_KEY{"nPulses"};
  static constexpr const char* RANGE_KEY {"range"};
  static constexpr const char* HISTO_KEY{"histo"};
  static constexpr const char* NBINS_KEY {"nBins"};
  static constexpr const char* FIT_KEY{"fit"};
  static constexpr const char* CHANNEL_KEY{"channel"};
  static constexpr const char* ID_KEY{"ID"};

  // constants
  static constexpr const char* MASTER_NAME{"bridge"};
  static constexpr const char* ADC_NAME{"ADC"};
  static constexpr const char* OUTPUT_PLOTS_FNAME{"histos.pdf"};
  static constexpr const char* OUTPUT_CONFIG_FNAME{"config.json"};
  static constexpr const char* OUTPUT_PULSES_TREE{"pulse"};
  static constexpr const char* OUTPUT_INTEGRALS_TREE{"integrated"};

  experiment(const ptree& config)
      : conf_{EXPERIMENT_KEY, config, "defaults", NAME_KEY}
      , path_proxy_{conf_, OUTDIR_KEY}
      , master_{new bridge_type{MASTER_NAME, config}}
      , adc_{ADC_NAME, config, master_, conf_.get<std::string>(CALIBRATION_KEY)}
      , ofile_{make_filename(conf_.get<std::string>(OUTDIR_KEY),
                             conf_.model() + ".root").c_str(),
               "recreate"}
      , config_fname_{make_filename(conf_.get<std::string>(OUTDIR_KEY),
                                    OUTPUT_CONFIG_FNAME)}
      , plots_fname_{make_filename(conf_.get<std::string>(OUTDIR_KEY),
                                   OUTPUT_PLOTS_FNAME)}
      , name_{conf_.get<std::string>(NAME_KEY)}
      , n_pulses_{conf_.get<size_t>(std::string(SIGNAL_PATH) + N_PULSES_KEY)}
      , n_integrals_{
            conf_.get<size_t>(std::string(INTEGRAL_PATH) + N_PULSES_KEY)} {

    init_histos();
    init_integration();

    LOG_INFO(name_, "Experiment initialized");

    run();
  }

  ~experiment() {
    LOG_INFO(name_, "Experiment done, closing output files");
    ofile_.Close();

    ptree config;
    conf_.save(config);
    master_->save_settings(config);
    adc_.save_settings(config);
    write_json(config_fname_, config);
  }

  static void calibrate(ptree& config) {
    configuration conf{EXPERIMENT_KEY, config, "defaults", NAME_KEY};

    create_path_proxy path_proxy{conf, CALIBRATION_KEY};

    const std::string path{path_proxy.path};

    std::shared_ptr<bridge_type> master{new bridge_type{MASTER_NAME, config}};

    adc_type::measure_pedestal(ADC_NAME, config, master, path, 100);
    adc_type::calibrate_verniers(ADC_NAME, config, master, path);
  }

private:
  void run() {
    LOG_INFO(name_, "Measuring " + std::to_string(n_pulses_) + " pulses");
    measure_pulses();
    LOG_INFO(name_, "Measuring " + std::to_string(n_integrals_) +
                        " integrated pulses");
    measure_integrals();
  }

  void measure_pulses() {
    ofile_.cd();
    TTree t{OUTPUT_PULSES_TREE, OUTPUT_PULSES_TREE};
    int channel[4]{0};
    int event{0};
    int sample{0};
    t.Branch("event", &event, "event/I");
    t.Branch("sample", &sample, "sample/I");
    t.Branch("channel0", &channel[0], "channel0/I");
    t.Branch("channel1", &channel[1], "channel1/I");
    t.Branch("channel2", &channel[2], "channel2/I");
    t.Branch("channel3", &channel[3], "channel3/I");

    adc_type::buffer_type buf;
    for (size_t i{0}; i < n_pulses_; ++i) {
      master_->wait_for_irq();
      adc_.read_pulse(buf);
      for (unsigned j{0}; j < buf.size(); ++j) {
        event = i;
        sample = j;
        for (unsigned k{0}; k < 4; ++k) {
          channel[k] = buf.get(k, j);
        }
        t.Fill();
      }
    }
    t.Write();
  }

  void measure_integrals() {
    ofile_.cd();
    TTree t{OUTPUT_INTEGRALS_TREE, OUTPUT_INTEGRALS_TREE};
    int channel[4]{0};
    t.Branch("channel0", &channel[0], "channel0/I");
    t.Branch("channel1", &channel[1], "channel1/I");
    t.Branch("channel2", &channel[2], "channel2/I");
    t.Branch("channel3", &channel[3], "channel3/I");

    TH1I* histos[4];
    for (size_t i{0}; i < 4; ++i) {
      histos[i] =
          new TH1I(channel_names_[i].c_str(), PMT_names_[i].c_str(),
                   histo_nbins_[i], histo_ranges_[i].first, histo_ranges_[i].second);
    }

    adc_type::buffer_type buf;
    for (size_t i{0}; i < n_integrals_; ++i) {
      master_->wait_for_irq();
      adc_.read_pulse(buf);
      for (unsigned k{0}; k < 4; ++k) {
        channel[k] = -buf.integrate(k, integration_ranges_[k]);
        histos[k]->Fill(channel[k]);
      }
      t.Fill();
    }
    t.Write();

    gStyle->SetOptFit(1);
    TCanvas c("ADC chan", "ADC chan", 800, 600);
    c.Divide(2, 2);

    for (int i = 0; i < 4; ++i) {
      c.cd(1 + i);
      // gPad->SetLogy();
      if (histo_fit_[i] != "none") {
        histos[i]->Fit(histo_fit_[i].c_str(), "Q");
      }
      histos[i]->Draw();
    }
    c.Print(plots_fname_.c_str());
  }

  void init_histo_names() {
    auto position = adc_.conf().get_vector<std::string>(CHANNEL_KEY);
    if (position.size() != 4) {
      throw conf_.value_error(CHANNEL_KEY, stringify(position));
    }
    for (size_t i = 0; i < 4; ++i) {
      position_names_[i] = position[i];
      channel_names_[i] = "channel " + std::to_string(i);
      PMT_names_[i] = channel_names_[i];
      if (position[i] == "none") {
        PMT_names_[i] = "none";
      } else {
        auto name = conf_.get_optional<std::string>(
            std::string(PMT_PATH) + position[i] + "." + ID_KEY);
        if (name) {
          PMT_names_[i] = *name + " (" + position[i] + ")";
        }
      }
    }
  }
  void init_histos() {
    init_histo_names();

    const std::string def_path {std::string(INTEGRAL_PATH) + HISTO_KEY + "."};
    for (size_t i = 0; i < 4; ++i) {
      const std::string path{PMT_PATH + position_names_[i] + "." + HISTO_KEY +
                             "."};
      auto nbins = conf_.get_optional<size_t>(path + NBINS_KEY);
      if (nbins) {
        histo_nbins_[i] = *nbins;
      } else {
        histo_nbins_[i] = conf_.get<size_t>(def_path + NBINS_KEY);
      }
      auto range = get_optional_range<int>(path + RANGE_KEY);
      if (range) {
        histo_ranges_[i] = *range;
      } else {
        histo_ranges_[i] = get_range<int>(def_path + RANGE_KEY);
      }
      auto fit = conf_.get_optional<std::string>(path + FIT_KEY);
      if (fit) {
        histo_fit_[i] = *fit;
      } else {
        histo_fit_[i] = conf_.get<std::string>(def_path + FIT_KEY);
      }
    }
  }

  void init_integration() {
    const std::string def_path {std::string(INTEGRAL_PATH) + RANGE_KEY};
    for (size_t i = 0; i < 4; ++i) {
      const std::string path{PMT_PATH + position_names_[i] + "." + RANGE_KEY};
      auto range = get_optional_range<size_t>(path);
      if (range) {
        integration_ranges_[i] = *range;
      } else {
        integration_ranges_[i] = get_range<size_t>(def_path);
      }
    }
  }

  template <class T>
  optional<std::pair<T, T>> get_optional_range(const std::string& key) {
    auto range = conf_.get_optional_vector<T>(key);
    if (range) {
      if (range->size() != 2) {
        throw conf_.translation_error(key, stringify(*range));
      }
      return {{(*range)[0], (*range)[1]}};
    }
    return {};
  }
  template <class T>
  std::pair<T, T> get_range(const std::string& key) {
    auto range = get_optional_range<T>(key);
    if (!range) {
      throw conf_.key_error(key);
    }
    return *range;
  }

  configuration conf_;
  create_path_proxy path_proxy_;
  std::shared_ptr<bridge_type> master_;
  adc_type adc_;
  TFile ofile_;

  const std::string config_fname_;
  const std::string plots_fname_;
  const std::string name_;

  const size_t n_pulses_;
  const size_t n_integrals_;

  std::array<std::pair<size_t, size_t>, 4> integration_ranges_;

  std::array<std::pair<int, int>, 4> histo_ranges_;
  std::array<size_t, 4> histo_nbins_;
  std::array<std::string, 4> histo_fit_;
  std::array<std::string, 4> channel_names_;
  std::array<std::string, 4> position_names_;
  std::array<std::string, 4> PMT_names_;
};

int main(int argc, char* argv[]) {

  // suppress ROOT signal handling
  root_suppress_signals();

  global::logger.set_level(log_level::JUNK2);

  try {
    // calibration
    if (argc == 3 && argv[2] == CALIBRATION_CMD) {
      LOG_INFO("main", "Generating calibrations");

      ptree config;
      read_json(argv[1], config);

      experiment::calibrate(config);

      LOG_INFO("main", "Calibrations complete, exiting");

      return 0;
    }

    tassert(argc == 2, "One command line parameter required (output file)");

    LOG_INFO("main", "Running PMT experiment");
    ptree config;
    read_json(argv[1], config);

    experiment ex{config};
  } catch (exception& e) {
    LOG_ERROR(e.type(), e.what());
    return 1;
  } catch (std::exception& e) {
    LOG_ERROR("std::exception", e.what());
    return -1;
  }

  LOG_INFO("main", "ALL DONE");
  return 0;
}
