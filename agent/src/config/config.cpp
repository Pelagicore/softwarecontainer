/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

#include "config.h"

namespace softwarecontainer {

// Illegal values used as initial command line option values to check if user
// set them or not
const std::string Config::SC_CONFIG_PATH_INITIAL_VALUE = "";
const int Config::PRELOAD_COUNT_INITIAL_VALUE = -1;
const bool Config::KEEP_CONTAINERS_ALIVE_INITIAL_VALUE = false;
const int Config::SHUTDOWN_TIMEOUT_INITIAL_VALUE = -2;
const std::string Config::SERVICE_MANIFEST_DIR_INITIAL_VALUE = "";
const std::string Config::DEFAULT_SERVICE_MANIFEST_DIR_INITIAL_VALUE = "";
const bool Config::USE_SESSION_BUS_INITIAL_VALUE = false;

// Config group "SoftwareContainer"
const std::string Config::SC_GROUP = "SoftwareContainer";

// Config keys for SoftwareContainer group
const std::string Config::PRELOAD_COUNT_KEY = "preload-count";
const std::string Config::KEEP_CONTAINERS_ALIVE_KEY = "keep-containers-alive";
const std::string Config::USE_SESSION_BUS_KEY = "session-bus";
const std::string Config::SHUTDOWN_TIMEOUT_KEY = "shutdown-timeout";
const std::string Config::SHARED_MOUNTS_DIR_KEY = "shared-mounts-dir";
const std::string Config::LXC_CONFIG_PATH_KEY = "deprecated-lxc-config-path";
const std::string Config::SERVICE_MANIFEST_DIR_KEY = "service-manifest-dir";
const std::string Config::DEFAULT_SERVICE_MANIFEST_DIR_KEY = "default-service-manifest-dir";


// TODO: Remove this constructor if we actually always will want to specify mandatory and deps?
Config::Config(std::unique_ptr<ConfigLoaderAbstractInterface> loader,
               std::unique_ptr<ConfigDefaults> defaults,
               const std::map<std::string, std::string> &stringOptions,
               const std::map<std::string, int> &intOptions,
               const std::map<std::string, bool> &boolOptions) :
    m_loader(std::move(loader)),
    m_defaults(std::move(defaults)),
    m_stringOptions(stringOptions),
    m_intOptions(intOptions),
    m_boolOptions(boolOptions)
{
    loadConfig();
}

Config::Config(std::unique_ptr<ConfigLoaderAbstractInterface> loader,
               std::unique_ptr<ConfigDefaults> defaults,
               MandatoryConfigs &mandatory,
               std::vector<std::pair<std::string, std::vector<std::string>>> &dependencies,
               const std::map<std::string, std::string> &stringOptions,
               const std::map<std::string, int> &intOptions,
               const std::map<std::string, bool> &boolOptions) :
    m_loader(std::move(loader)),
    m_defaults(std::move(defaults)),
    m_stringOptions(stringOptions),
    m_intOptions(intOptions),
    m_boolOptions(boolOptions)
{
    loadConfig();

    /* Checking for mandatory config group is only relevant for the main config source
     */
    for (std::string group : mandatory.groups()) {
        log_debug() << "Checking for mandatory config group: \"" << group << "\"";
        if (!m_config->has_group(group)) {
            log_error() << "Missing mandatory config group: \"" + group + "\"";
            throw ConfigMandatoryError("Missing mandatory config group: \"" + group + "\"");
        }
    }

    /* Check that each mandatory config is present in one of the sources using the normal
     * retrieval methods in order to preserve the priority order.
     */
    for (std::tuple<std::string, std::string, ConfigType> config : mandatory.configs()) {
        std::string group = std::get<0>(config);
        std::string key = std::get<1>(config);
        ConfigType type = std::get<2>(config);

        log_debug() << "Checking for presence of mandatory config: \"" << key << "\"";
        /* TODO: The below only checks for presence, it should also add the values to something
         *       that can be used for retrieval later. */
        try {
            switch (type) {
                case ConfigType::String:
                    getStringValue(group, key);
                    break;
                case ConfigType::Integer:
                    getIntegerValue(group, key);
                    break;
                case ConfigType::Boolean:
                    getBooleanValue(group, key);
                    break;
                default:
                    break;
            }
        } catch (ConfigError &error) {
            log_error() << "Missing mandatory config: \"" << key << "\"";
            throw ConfigMandatoryError("Missing mandatory config: \"" + key + "\"");
        }
    }
}

void Config::loadConfig()
{
    try {
        m_config = m_loader->loadConfig();
    } catch (Glib::Error &error) {
        throw ConfigError();
    }
}

std::string Config::getStringValue(const std::string &group, const std::string &key) const
{
    return getValue<std::string>(group, key, m_stringOptions);
}

int Config::getIntegerValue(const std::string &group, const std::string &key) const
{
    return getValue<int>(group, key, m_intOptions);
}

bool Config::getBooleanValue(const std::string &group, const std::string &key) const
{
    return getValue<bool>(group, key, m_boolOptions);

}

template<typename T>
T Config::getValue(const std::string &group, const std::string &key, const std::map<std::string, T> &options) const
{
    // Check if this key has been explicitly set and if so, return that value
    typename std::map<std::string, T>::const_iterator position = options.find(key);
    if (position != options.end()) {
        // If the end wasn't reached, the value is where the iterator is now
        log_debug() << "Config \""
                    << group << "::" << key
                    << "\" found in explicitly set options. Value: \""
                    << position->second << "\"";
        return position->second;
    }

    T value;

    // The value was not explicitly set so look for value in the config
    if (!m_config->has_group(group)) {
        // Incorrect group is a configuration error, don't continue to fall back to default
        log_error() << "Incorrect config group: \"" << group << "\"";
        throw ConfigError();
    }

    if (m_config->has_key(group, key)) {
        try {
            value = getGlibValue<T>(group, key);
            log_debug() << "Config \""
                        << group << "::" << key
                        << "\" found in config file. Value: \""
                        << value << "\"";
            return value;
        } catch (Glib::Error &error) {
            log_error() << "Could not parse string value: \"" << error.what() << "\"";
            throw ConfigError();
        }
    }

    // We should always have defaults set, but check just to be sure.
    if (!m_defaults) {
        log_error() << "No config defaults set, cannot continue";
        throw ConfigError();
    }

    // Value has not been set at all, fall back on default
    try {
        value = m_defaults->getValue<T>(key);
        log_debug() << "Config \""
                    << group << "::" << key
                    << "\" is falling back on default. Value: \""
                    << value << "\"";
    } catch (std::exception &error) {
        log_error() << "No config default found for config \"" << group << "::" << key << "\"";
        throw ConfigError();
    }

    return value;
}

template<>
std::string Config::getGlibValue(const std::string &group, const std::string &key) const
{
    return m_config->get_string(Glib::ustring(group), Glib::ustring(key));
}

template<>
bool Config::getGlibValue(const std::string &group, const std::string &key) const
{
    return m_config->get_boolean(Glib::ustring(group), Glib::ustring(key));
}

template<>
int Config::getGlibValue(const std::string &group, const std::string &key) const
{
    return m_config->get_integer(Glib::ustring(group), Glib::ustring(key));
}

} // namespace softwarecontainer
