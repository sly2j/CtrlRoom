#ifndef CTRLROOM_BOARD_LOADED
#define CTRLROOM_BOARD_LOADED

#include <ctrlroom/util/configuration.hpp>
#include <string>

namespace ctrlroom {

// hardware board base class
//
// CONFIGURATION FILE OPTIONS:
//      * board model: <identifier>.model (cf. MODEL_KEY)
//      * default section path is "defaults" (cf. DEFAULTS_PATH)
//      * (optional) board default settings: defaults.<model>
class board {
public:
  constexpr static const char *DEFAULTS_PATH{ "defaults" };
  constexpr static const char *MODEL_KEY{ "model" };

  board(const std::string &identifier, const ptree &settings);

  void save_settings(ptree &settings) const;

  const std::string &name() const { return name_; }

  const configuration &conf() const { return conf_; }
  configuration &conf() { return conf_; }

protected:
  configuration conf_;
  const std::string name_;
};
}

#endif
