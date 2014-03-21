#ifndef CTRLROOM_CONTROLER_LOADED
#define CTRLROOM_CONTROLER_LOADED

#include <string>

namespace ctrlroom {


    // DAQ Controller base class
    template <class CtrlImpl>
        class controller {
            public:
                typedef CtrlImpl controller_type;

                controller(const std::string name)
                    : name_ {name} {}

                // destructor automatically closes the DAQ session
                ~controller() {
                    impl().close();
                }

                // unique resource (not copyable or assignable)
                controller(const controller&) = delete;
                controller& operator= (const controller&) = delete;

                const std::string& name() {
                    return name_;
                }

            protected:
                void close();

            private:
                controller_type& impl() {
                    return static_cast<controller_type&>(*this);
                }

                const std::string name_;
        };



}

#endif
