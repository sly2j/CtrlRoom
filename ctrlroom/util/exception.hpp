#ifndef CTRLROOM_UTILITY_EXCEPTION_LOADED
#define CTRLROOM_UTILITY_EXCEPTION_LOADED

#include <exception>
#include <string>

namespace ctrlroom {

    class exception : 
        public std::exception
    {
    public:
        exception(const std::string& msg)
            : msg_ {msg} {}

        virtual const char* what() const throw () {
            return msg_.c_str();
        }
        virtual ~exception() throw () {}
    private:
        std::string msg_;
    };

}

#endif
