#ifndef CTRLROOM_UTILITY_LOGGER_LOADED
#define CTRLROOM_UTILITY_LOGGER_LOADED

#include <ctime>
#include <ostream>
#include <iostream> // for std::cout
#include <string>
#include <array>
#include <mutex>

namespace ctrlroom {

class log_handler;

// default global logger
namespace global {
extern log_handler logger;
}

enum class log_level : unsigned {
  NOTHING = 0,
  CRITICAL = 1,
  ERROR = 2,
  WARNING = 3,
  INFO = 4,
  DEBUG = 5,
  JUNK = 6,
  JUNK2 = 7
};
constexpr std::array<const char*, 8> LOG_LEVEL_NAMES{
    "nothing", "critical", "error", "warning",
    "info",    "debug",    "junk",  "junk2"};

// global logger function
template <log_level level>
void log(const std::string& mtitle, const std::string& mtext,
         log_handler& logger = global::logger);
}

// PREPROCESSOR macros to actually call the logger.
// Strongly prefered over calling below loggers directly, as in the macros,
// mtitle and mtext (which might be complex expressions) are only evaluated
// *after* the log_level check.
// This is *significantly* faster than calling log<LEVEL>(mtitle, mtext)
// directly in the code, because the compiler isn't allowed to this for us.
// So, don't use any assignemts in the arguments, they
// won't always work ;-) (also, don't do this anyway!).
#define LOG_CRITICAL(mtitle, mtext)                                            \
  if (ctrlroom::global::logger.level() >= ctrlroom::log_level::CRITICAL) {     \
    ctrlroom::log<log_level::CRITICAL>((mtitle), (mtext));                     \
  }
#define LOG_ERROR(mtitle, mtext)                                               \
  if (ctrlroom::global::logger.level() >= ctrlroom::log_level::ERROR) {        \
    ctrlroom::log<ctrlroom::log_level::ERROR>((mtitle), (mtext));              \
  }
#define LOG_WARNING(mtitle, mtext)                                             \
  if (ctrlroom::global::logger.level() >= ctrlroom::log_level::WARNING) {      \
    ctrlroom::log<ctrlroom::log_level::WARNING>((mtitle), (mtext));            \
  }
#define LOG_INFO(mtitle, mtext)                                                \
  if (ctrlroom::global::logger.level() >= ctrlroom::log_level::INFO) {         \
    ctrlroom::log<ctrlroom::log_level::INFO>((mtitle), (mtext));               \
  }
#define LOG_DEBUG(mtitle, mtext)                                               \
  if (ctrlroom::global::logger.level() >= ctrlroom::log_level::DEBUG) {        \
    ctrlroom::log<ctrlroom::log_level::DEBUG>((mtitle), (mtext));              \
  }
#define LOG_JUNK(mtitle, mtext)                                                \
  if (ctrlroom::global::logger.level() >= ctrlroom::log_level::JUNK) {         \
    ctrlroom::log<ctrlroom::log_level::JUNK>((mtitle), (mtext));               \
  }
#define LOG_JUNK2(mtitle, mtext)                                               \
  if (ctrlroom::global::logger.level() >= ctrlroom::log_level::JUNK2) {        \
    ctrlroom::log<ctrlroom::log_level::JUNK2>((mtitle), (mtext));              \
  }

namespace ctrlroom {

// log handler class designed for global usage,
// threading secure
class log_handler {
private:
  typedef std::mutex mutex_type;
  typedef std::lock_guard<mutex_type> lock_type;

public:
  log_handler(const log_level level = log_level::INFO,
              std::ostream& sink = std::cout);

  log_level level() const {
    lock_type lock{mutex_};
    return level_;
  }

  void set_level(const log_level level);
  void set_level(unsigned ulevel);

  void operator()(const log_level mlevel, const std::string& mtitle,
                  const std::string& mtext) {
    lock_type lock{mutex_};
    if (mlevel > level_)
      return;
    time_t rt;
    time(&rt);
    sink_ << "[" << rt << ", " << mtitle << ", "
          << LOG_LEVEL_NAMES[static_cast<unsigned>(mlevel)] << "] " << mtext
          << std::endl;
  }

private:
  log_level level_;
  std::ostream& sink_;
  mutable mutex_type mutex_;
};

template <log_level level>
void log(const std::string& mtitle, const std::string& mtext,
         log_handler& logger) {
  logger(level, mtitle, mtext);
}
}

#endif
