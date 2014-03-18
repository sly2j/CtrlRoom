#include "logger.hpp"

namespace ctrlroom {

    void 
    log_handler::set_level(const log_level level) {
        lock_t lock {mutex_};
        level_ = level;
    }

    void
    log_handler::set_level(unsigned ulevel) {
        lock_t lock {mutex_};
        if (ulevel >= LOG_LEVEL_NAMES.size()) {
            ulevel = LOG_LEVEL_NAMES.size() - 1;
        }
        level_ = static_cast<log_level>(ulevel);
    }

    namespace global {
        log_handler logger {};
    }

}
