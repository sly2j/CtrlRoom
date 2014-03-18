#ifndef CTRLROOM_ASSERT_LOADED
#define CTRLROOM_ASSERT_LOADED

#include <CtrlRoom/util/logger.hpp>
#include <CtrlRoom/util/exception.hpp>
#include <CR/iostream.hpp>
#include <string>


// throwing assert
#define tassert(condition, msg) \
   ctrlroom::tassert_impl((condition), #condition, __FILE__, __LINE__, msg)

namespace ctrlroom {
    inline 
    void tassert_impl(bool ok, 
                      const std::string& condition,
                      const std::string& location,
                      const int line,
                      const std::string& msg) {
        if (!ok)
        {
            LOG_ERROR(location, 
                                "l" + CR::io::lexical_cast<std::string>(line)
                                + ": assert(" + condition + ") failed");
            throw exception{msg};
        }
    }
}

#endif
