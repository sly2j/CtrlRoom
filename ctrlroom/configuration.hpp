#ifndef CTRLROOM_CONFIGURATION_LOADED
#define CTRLROOM_CONFIGURATION_LOADED

#include <ctrlroom/util/logger.hpp>
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
                        const std::string& path)
                    : configuration_error(
                            "Invalid configuration path " + path, 
                            "configuration_path_error") {}
        };
    class configuration_key_error
        : public configuration_error {
            public:
                configuration_key_error(
                        const std::string& key,
                        const std::string& settings_path,
                        const std::string& defaults_path)
                    : configuration_error(
                        "Cannot find " + key 
                        + " (in " + settings_path
                        + " or " + defaults_path + ")", 
                        "configuration_key_error") {}
        };

    class configuration_value_error
        : public configuration_error {
            public:
                configuration_value_error(
                        const std::string& key,
                        const std::string& value,
                        const std::string& settings_path,
                        const std::string& defaults_path)
                    : configuration_error(
                            "Invalid value " + value 
                            + " for key " + key
                            + " (in " + settings_path
                            + " or " + defaults_path + ")", 
                            "configuration_value_error") {}
        };
    class configuration_translation_error
        : public configuration_error {
            public:
                configuration_translation_error(
                        const std::string& key,
                        const std::string& value,
                        const std::string& settings_path,
                        const std::string& defaults_path)
                    : configuration_error(
                            "Unable to translate value " + value 
                            + " for key " + key
                            + " (in " + settings_path
                            + " or " + defaults_path + ")", 
                            "configuration_translation_error") {}
        };
    
    // CRTP configuration handler mixin
    //
    // defines the following configuration file settings:
    //      * board model: <path>.model
    // default settings:
    //      * default section root <defaults_root> (def: "defaults")
    //      * (optional) board default settings: <defaults_root>.<model>
    template <Board>
        class configuration {
            public:
                typedef Board board_type;

                configuration(
                        const ptree& settings,
                        const std::string& path,
                        const std::string& defaults_root = "defaults")
                    : settings_path_ {path} {

                        try {
                            load_settings();
                        } catch (ptree_bad_path& e) {
                            throw path_error(e.path<std::string>());
                        } catch (ptree_error& e) {
                            // shouldn't happen
                            throw configuration_error("Processing error", e.what());
                        }
                    }

                // load the settings from a given ptree
                void load_settings(
                        const ptree& in_conf) {

                    settings_ = settings.get_child(path);
                    LOG_INFO(path, "Loading settings");

                    auto model = settings.get_optional<std::string>("model");
                    if (!model) {
                        throw configuration_error(
                                "key 'model' has to be set in " 
                                + path);
                    }
                    defaults_path_ = defaults_root + "." + *model;
                    auto def = settings.get_child_optional(defaults_path_);
                    if (def) {
                        LOG_INFO(path, *model + " defaults found.");
                        defaults_ = *def;
                    } else {
                        LOG_INFO(path, 
                                "No default settings provided for this board.");
                    }

                    board().process_settings();
                }

                // store the settings in the give ptree
                // The defaults are only exported if not yet present.
                void save_settings(
                        ptree& out_conf) const {
                    out_conf.put_child(settings_path_, settings_);
                    LOG_INFO(settings_path_, "Settings saved.");
                    if (!out_conf.get_child_optional(defaults_path_))
                        out_conf.put_child(defaults_path_, defaults_);
                        LOG_INFO(defaults_path_, "Settings saved.");
                    }
                }

                // process the settings and defaults
                // overwrite this function in child classes to actually
                // handle the configuration
                void process_settings() {
                    ; // do nothing
                }

            protected:
                // Three functions to get a setting by its key:
                //
                // 1. optional version
                template <class T>
                    optional<T> get_optional_setting(
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
                // 2. Throwing version
                template <class T>
                    T get_setting(
                            const std::string& key) const {
                        auto s = get_optional_setting(key);
                        if (!s) {
                            throw key_error(key);
                        }
                        return *s;
                    }
                // 3. default-value version
                //      if default_value is needed, it is automatically added to
                //      the default configurations for this board
                template <class T>
                    T get_setting(
                            const std::string& key, 
                            const T& default_value) {
                        auto s = get_optional_setting(key);
                        if (!s) 
                        {
                            defaults_.put(key, default_value);
                            return default_value;
                        }
                        return *s;
                    }

                // Helper functions to raise exceptions
                configuration_path_error path_error(
                        const std::string& path) const {
                    return configuration_path_error(path);
                }
                configuration_key_error key_error(
                        const std::string& key) const {
                    return configuration_key_error(key, 
                                                   settings_path_, 
                                                   defaults_path_);
                }
                configuration_value_error value_error(
                        const std::string& key,
                        const std::string& value) const {
                    return configuration_value_error(key,
                                                     value,
                                                     settings_path_,
                                                     defaults_path_);
                }
                configuration_translation_error translation_error(
                        const std::string& key,
                        const std::string& value) {
                    return configuration_value_error(key,
                                                     value,
                                                     settings_path_,
                                                     defaults_path_);
                }

            private:
                // access the actual (derived) board class
                board_type& board() {
                    return static_cast<board_type&>(*this);
                }

                // settings
                const std::string settings_path_;
                ptree settings_;
                // defaults
                defaults_path_;
                ptree defaults_;
        };

}

#endif
