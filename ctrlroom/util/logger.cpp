#include "logger.hpp"

namespace ctrlroom {
log_handler::log_handler(const log_level level, std::ostream& sink)
    : level_{level}, sink_(sink) {}

void log_handler::set_level(const log_level level) {
  lock_type lock{mutex_};
  level_ = level;
}

void log_handler::set_level(unsigned ulevel) {
  lock_type lock{mutex_};
  if (ulevel >= LOG_LEVEL_NAMES.size()) {
    ulevel = LOG_LEVEL_NAMES.size() - 1;
  }
  level_ = static_cast<log_level>(ulevel);
}

namespace global {
log_handler logger{};
}
}
