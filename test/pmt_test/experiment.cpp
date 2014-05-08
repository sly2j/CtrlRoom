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

#include <TCanvas.h>
#include <TH1I.h>

#include <boost/filesystem.hpp>

using namespace ctrlroom;

using bridge_type = vme::caen_bridge;
using adc_type = vme::caen_v1729a<bridge_type, vme::addressing_mode::A32,
                                  vme::transfer_mode::D32,   // single
                                  vme::transfer_mode::MBLT>; // BLT

const std::string CALIBRATION_CMD{ "calibrate" };

struct create_path_proxy {
  create_path_proxy(const configuration &conf, const std::string &path_key)
      : path{ conf.get<std::string>(path_key) } {
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
  static constexpr const char *EXPERIMENT_KEY{ "experiment" };
  static constexpr const char *NAME_KEY{ "name" };
  static constexpr const char *CALIBRATION_KEY{ "calibrationDirectory" };
  static constexpr const char *OUTDIR_KEY{ "outputDirectory" };
  static constexpr const char *N_PULSES_KEY{ "nPulses" };
  static constexpr const char *N_INTEGRALS_KEY{ "nIntegratedPulses" };
  static constexpr const char *INTEGRATION_RANGE_KEY{ "integrationRange" };
  static constexpr const char *HISTO_RANGE_KEY{ "histoRange" };

  // constants
  static constexpr const char *MASTER_NAME{ "bridge" };
  static constexpr const char *ADC_NAME{ "ADC" };
  static constexpr const char *OUTPUT_RESULTS_FNAME{ "pulses.root" };
  static constexpr const char *OUTPUT_PLOTS_FNAME{ "histos.pdf" };
  static constexpr const char *OUTPUT_CONFIG_FNAME{ "config.json" };
  static constexpr const char *OUTPUT_PULSES_TREE{ "pulse" };
  static constexpr const char *OUTPUT_INTEGRALS_TREE{ "integrated" };

  experiment(const ptree &config)
      : conf_{ EXPERIMENT_KEY, config, "defaults", NAME_KEY },
        path_proxy_{ conf_, OUTDIR_KEY },
        master_{ new bridge_type{ MASTER_NAME, config } },
        adc_{
          ADC_NAME, config, master_, conf_.get<std::string>(CALIBRATION_KEY)
        },
        ofile_{ make_filename(conf_.get<std::string>(OUTDIR_KEY),
                              OUTPUT_RESULTS_FNAME).c_str(),
                "recreate" },
        config_fname_{ make_filename(conf_.get<std::string>(OUTDIR_KEY),
                                     OUTPUT_CONFIG_FNAME) },
        plots_fname_{ make_filename(conf_.get<std::string>(OUTDIR_KEY),
                                    OUTPUT_PLOTS_FNAME) },
        name_{ conf_.get<std::string>(NAME_KEY) },
        n_pulses_{ conf_.get<size_t>(N_PULSES_KEY) },
        n_integrals_{ conf_.get<size_t>(N_INTEGRALS_KEY) } {
    auto range = conf_.get_vector<size_t>(INTEGRATION_RANGE_KEY);
    if (range.size() != 2) {
      throw conf_.translation_error(INTEGRATION_RANGE_KEY, stringify(range));
    }
    integration_range_ = { range[0], range[1] };

    auto range2 = conf_.get_vector<int>(HISTO_RANGE_KEY);
    if (range2.size() != 2) {
      throw conf_.translation_error(HISTO_RANGE_KEY, stringify(range));
    }
    histo_range_ = { range2[0], range2[1] };

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

  static void calibrate(ptree &config) {
    configuration conf{ EXPERIMENT_KEY, config, "defaults", NAME_KEY };

    create_path_proxy path_proxy{ conf, CALIBRATION_KEY };

    const std::string path{ path_proxy.path };

    std::shared_ptr<bridge_type> master{ new bridge_type{ MASTER_NAME,
                                                          config } };

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
    TTree t{ OUTPUT_PULSES_TREE, OUTPUT_PULSES_TREE };
    int channel[4]{ 0 };
    int event{ 0 };
    int sample{ 0 };
    t.Branch("event", &event, "event/I");
    t.Branch("sample", &sample, "sample/I");
    t.Branch("channel0", &channel[0], "channel0/I");
    t.Branch("channel1", &channel[1], "channel1/I");
    t.Branch("channel2", &channel[2], "channel2/I");
    t.Branch("channel3", &channel[3], "channel3/I");

    adc_type::buffer_type buf;
    for (size_t i{ 0 }; i < n_pulses_; ++i) {
      master_->wait_for_irq();
      adc_.read_pulse(buf);
      for (unsigned j{ 0 }; j < buf.size(); ++j) {
        event = i;
        sample = j;
        for (unsigned k{ 0 }; k < 4; ++k) {
          channel[k] = buf.get(k, j);
        }
        t.Fill();
      }
    }
    t.Write();
  }

  void measure_integrals() {
    ofile_.cd();
    TTree t{ OUTPUT_INTEGRALS_TREE, OUTPUT_INTEGRALS_TREE };
    int channel[4]{ 0 };
    t.Branch("channel0", &channel[0], "channel0/I");
    t.Branch("channel1", &channel[1], "channel1/I");
    t.Branch("channel2", &channel[2], "channel2/I");
    t.Branch("channel3", &channel[3], "channel3/I");

    TH1I *histos[4];
    const char *ch[]{ "channel0", "channel1", "channel2", "channel3" };
    for (size_t i{ 0 }; i < 4; ++i) {
      histos[i] = new TH1I(ch[i], ch[i], (n_integrals_) / 300,
                           histo_range_.first, histo_range_.second);
    }

    adc_type::buffer_type buf;
    for (size_t i{ 0 }; i < n_integrals_; ++i) {
      master_->wait_for_irq();
      adc_.read_pulse(buf);
      for (unsigned k{ 0 }; k < 4; ++k) {
        channel[k] = -buf.integrate(k, integration_range_);
        histos[k]->Fill(channel[k]);
      }
      t.Fill();
    }
    t.Write();

    TCanvas c("ADC chan", "ADC chan", 800, 600);
    c.Divide(2, 2);
    for (int i = 0; i < 4; ++i) {
      c.cd(1 + i);
      // gPad->SetLogy();
      histos[i]->Draw();
    }
    c.Print(plots_fname_.c_str());
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

  std::pair<size_t, size_t> integration_range_;
  std::pair<int, int> histo_range_;
};

int main(int argc, char *argv[]) {

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

    experiment ex{ config };
  }
  catch (exception &e) {
    LOG_ERROR(e.type(), e.what());
    return 1;
  }
  catch (std::exception &e) {
    LOG_ERROR("std::exception", e.what());
    return -1;
  }

  LOG_INFO("main", "ALL DONE");
  return 0;
}
