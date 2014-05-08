#include "board.hpp"

using namespace ctrlroom;

////////////////////////////////////////////////////////////////////////////////
// class board
////////////////////////////////////////////////////////////////////////////////
board::board(const std::string &identifier, const ptree &settings)
    : conf_{ identifier, settings, DEFAULTS_PATH, MODEL_KEY },
      name_{ identifier } {}

void board::save_settings(ptree &settings) const { conf_.save(settings); }
