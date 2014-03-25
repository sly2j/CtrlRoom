#ifndef CTRLROOM_CONFIGURATION_LOADED
#define CTRLROOM_CONFIGURATION_LOADED

#include <ctrlroom/util/exception.hpp>

#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/optional.hpp>

namespace ctrlroom {
    
    using boost::property_tree::ptree;
    using boost::optional;
    using boost::property_tree::ptree_error;
    using boost::property_tree::ptree_bad_path;
    using boost::property_tree::ptree_bad_data;

    class configuration_error;
    class configuration_path_error;
    class configuration_key_error;
    class configuration_value_error;
    class configuration_translation_error;
    
    // configuration handler
    //
    // The configuration constructor takes a Board identifier string
    // as argument. This string should match the settings path
    // in the associated settings ptree.
    //
    // CONFIGURATION FILE OPTIONS:
    //      * board model: <identifier>.model (cf. MODEL_KEY)
    //      * default section root is "defaults" (cf. DEFAULTS_KEY)
    //      * (optional) board default settings: defaults.<model>
    class configuration {
        public:
            constexpr static char* MODEL_KEY {"model"};
            constexpr static char* DEFAULTS_KEY {"defaults"};

            configuration(
                    const std::string& identifier,
                    const ptree& settings);

            // load the settings from a given ptree
            void load(const ptree& in_conf);

            // store the settings in the give ptree
            // The defaults are only exported if not yet present.
            void save(ptree& out_conf) const;

            // Three functions to get a setting by its key:
            //
            // 1. optional version
            template <class T>
                optional<T> get_optional(const std::string& key) const;
            // 2. Throwing version
            template <class T>
                T get(const std::string& key) const;
            // 3. default-value version
            //      if default_value is needed, it is automatically added to
            //      the default configurations for this board
            template <class T>
                T get(const std::string& key, 
                      const T& default_value);

            // Helper functions to construct exceptions
            configuration_path_error path_error(const std::string& path) const;
            configuration_key_error key_error(const std::string& key) const;
            configuration_value_error value_error(
                    const std::string& key,
                    const std::string& value) const;
            configuration_translation_error translation_error(
                    const std::string& key,
                    const std::string& value);
        
        private:

            // settings
            const std::string settings_path_;
            ptree settings_;
            // defaults
            defaults_path_;
            ptree defaults_;
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////
namespace ctrlroom
{
    //configuration
    template <class T>
        optional<T> configuration::get_optional(
                const std::string& key) const {
            try {
                auto s = settings_.get_optional<T>(key);
                if (!s) {
                    s = defaults_.get_optional<T>(key);
                }
                return s;
            } catch (ptree_bad_data& e) {
                throw translation_error(key, e.data<std::string>());
            }
        }
    template <class T>
        T configuration::get(
                const std::string& key) const {
            auto s = get_optional(key);
            if (!s) {
                throw key_error(key);
            }
            return *s;
        }
    template <class T>
        T configuration::get(
                const std::string& key, 
                const T& default_value) {
            auto s = get_optional(key);
            if (!s) 
            {
                defaults_.put(key, default_value);
                return default_value;
            }
            return *s;
        }

    // exceptions
    class configuration_error
        : public ctrlroom::exception {
            public:
                configuration_error(
                        const std::string& msg,
                        const std::string& type="configuration_error")
                    : ctrlroom::exception(msg, type) {} 
        };

    class configuration_path_error
        : public configuration_error {
            public:
                configuration_path_error(
                        const std::string& path);
        };
    class configuration_key_error
        : public configuration_error {
            public:
                configuration_key_error(
                        const std::string& key,
                        const std::string& settings_path,
                        const std::string& defaults_path);
        };

    class configuration_value_error
        : public configuration_error {
            public:
                configuration_value_error(
                        const std::string& key,
                        const std::string& value,
                        const std::string& settings_path,
                        const std::string& defaults_path);
        };
    class configuration_translation_error
        : public configuration_error {
            public:
                configuration_translation_error(
                        const std::string& key,
                        const std::string& value,
                        const std::string& settings_path,
                        const std::string& defaults_path);
        };

}

#endif
