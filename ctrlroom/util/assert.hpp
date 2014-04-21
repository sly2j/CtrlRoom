#ifndef CTRLROOM_ASSERT_LOADED
#define CTRLROOM_ASSERT_LOADED

#include <ctrlroom/util/logger.hpp>
#include <ctrlroom/util/exception.hpp>
#include <string>


// throwing assert
#define tassert(condition, msg) \
    if (!(condition)) { \
        ctrlroom::tassert_impl(#condition, __FILE__, __LINE__, msg); \
    }

namespace ctrlroom {
    inline 
    void tassert_impl(const std::string& condition,
                      const std::string& location,
                      const int line,
                      const std::string& msg) {
        LOG_ERROR(location, 
                            "l" + std::to_string(line)
                            + ": assert(" + condition + ") failed");
        throw exception{msg, "assert"};
    }
}

#endif
