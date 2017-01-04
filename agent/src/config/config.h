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

#pragma once

#include <string>
#include <exception>
#include <glibmm.h>

#include "softwarecontainer-common.h"
#include "configloaderabstractinterface.h"
#include "configdefaults.h"
#include "mandatoryconfigs.h"

namespace softwarecontainer {

class ConfigError : public std::exception
{
public:
    ConfigError():
        m_message("SoftwareContainer configuration error")
    {
    }

    ConfigError(const std::string &message):
        m_message(message)
    {
    }

    virtual const char *what() const throw()
    {
        return m_message.c_str();
    }

protected:
    std::string m_message;
};


class ConfigDependencyError : public ConfigError
{
public:
    ConfigDependencyError():
        ConfigError("Configuration error - missing config dependency")
    {
    }

    ConfigDependencyError(const std::string &message):
        ConfigError(message)
    {
    }

    virtual const char *what() const throw()
    {
        return ConfigError::m_message.c_str();
    }
};


class ConfigMandatoryError : public ConfigError
{
public:
    ConfigMandatoryError():
        ConfigError("Configuration error - missing mandatory config")
    {
    }

    ConfigMandatoryError(const std::string &message):
        ConfigError(message)
    {
    }

    // TODO: Can these be removed since we initialize the parent with a message anyway?
    virtual const char *what() const throw()
    {
        return ConfigError::m_message.c_str();
    }
};


/**
 * @brief Represents the type of a config value
 *
 * This is used to specify the type of the a value when mandatory configs are
 * defined. This information is later used to know how a key should be used to
 * retrive a value, e.g. what type specific methods can be used.
 */
enum class ConfigType
{
    String,
    Integer,
    Boolean
};


/**
 * @brief Represents the SoftwareContainer main config
 *
 * A 'config' is a key-value pair, a 'config source' is somewhere that these key-value pairs
 * are defined.
 *
 * There are three different config sources:
 *   * Command line options
 *   * Main config file
 *   * Defaults
 *
 * Some configs are optional and some are mandatory. A config can become mandatory if
 * some other specified config depends on it. E.g. if an optional config in the main config
 * file is uncommented and thus used, it might mean that other configs are mandatory
 * as a consequence. A mandatory config must always be present in the command line options
 * or in the main config file.
 *
 * In the main config file, there are also 'confgig groups' which some might be mandatory and
 * others not. Groups does not exist in the other config sources.
 *
 * NOTE: It is an error if a mandatory config is not found in any config source when Config
 *       is initialized.
 *
 * NOTE: It is en error if a dependency to a config is not found in any config source when
 *       Config is initialized.
 *
 * NOTE: It is an error if a mandatory config group is not found in the main config file when
 *       Config is initialized.
 *
 * Config sources have an order of priority:
 *   1 - Command line options
 *   2 - Main config file
 *   3 - Defaults
 *
 * The config sources as considered in the above order, i.e. first command line options etc.
 * As soon as a config is found in a source, the value is returned and less prioritized sources
 * are ignored.
 *
 * NOTE: It is an error if a config is requested and not found in any source.
 */
class Config
{

LOG_DECLARE_CLASS_CONTEXT("CFG", "SoftwareContainer general config");

public:
    /**
     * @brief Constructor
     *
     * The optional params takes any potential explicitly set configs, e.g. read
     * as command line options. These values will take precedence over any
     * existing values with the same key in the static configuration.
     *
     * If no configs are found among the explicit set configs or in the config file
     * provided by the loader, the default values will be used as a last resort.
     *
     * If no explicit configs are to be used, this Constructor can be called
     * by passing only the 'loader' and 'defaults' arguments.
     * Otherwise all parameters needs to be passed.
     *
     * It is an error to not provide default configuration values. Not finding values
     * in any of the other sources, i.e. explicit or config file, will not cause an
     * error unless there are problems parsing.
     *
     * @param loader An interface to a loader which provides the underlying
     *               config to this class
     * @param defaults An interface to default configs
     * @param stringOptions A string:string map with config keys and corresponding values
     * @param intOptions A string:int map with config keys and corresponding values
     * @param boolOptions A string:bool map with config keys and corresponding values
     *
     * @throws ConfigError On error
     */
    Config(std::unique_ptr<ConfigLoaderAbstractInterface> loader,
           std::unique_ptr<ConfigDefaults> defaults,
           const std::map<std::string, std::string> &stringOptions = std::map<std::string, std::string>(),
           const std::map<std::string, int> &intOptions = std::map<std::string, int>(),
           const std::map<std::string, bool> &boolOptions = std::map<std::string, bool>());

    /**
     * TODO: Find a way to reuse parts of above comment
     *
     * @param mandatory A reference to a MandatoryConfigs object that specifies what configs
     *                  will be considered mandatory.
     * @param dependencies A list of dependee keys mapping to lists of dependency keys
     */
    Config(std::unique_ptr<ConfigLoaderAbstractInterface> loader,
        std::unique_ptr<ConfigDefaults> defaults,
        MandatoryConfigs &mandatory,
        std::vector<std::pair<std::string, std::vector<std::string>>> &dependencies,
        const std::map<std::string, std::string> &stringOptions = std::map<std::string, std::string>(),
        const std::map<std::string, int> &intOptions = std::map<std::string, int>(),
        const std::map<std::string, bool> &boolOptions = std::map<std::string, bool>());

    /**
     * @brief getStringValue Get a config value of type string
     *
     * @param group The name of the config group the value belongs to
     * @param key The key for the config value
     *
     * @return A string with the config value
     */
    std::string getStringValue(const std::string &group, const std::string &key) const;

    /**
     * @brief getIntegerValue Get a config value of type integer
     *
     * @param group The name of the config group the value belongs to
     * @param key The key for the config value
     *
     * @return An int with the config value
     */
    int getIntegerValue(const std::string &group, const std::string &key) const;

    /**
     * @brief getBooleanValue Get a config value of type boolean
     *
     * @param group The name of the config group the value belongs to
     * @param key The key for the config value
     *
     * @return A bool with the config value
     */
    bool getBooleanValue(const std::string &group, const std::string &key) const;

    // Illegal values used as initial command line option values to check if user
    // set them or not
    static const std::string SC_CONFIG_PATH_INITIAL_VALUE;
    static const int PRELOAD_COUNT_INITIAL_VALUE;
    static const bool KEEP_CONTAINERS_ALIVE_INITIAL_VALUE;
    static const int SHUTDOWN_TIMEOUT_INITIAL_VALUE;
    static const std::string SERVICE_MANIFEST_DIR_INITIAL_VALUE;
    static const std::string DEFAULT_SERVICE_MANIFEST_DIR_INITIAL_VALUE;
    static const bool USE_SESSION_BUS_INITIAL_VALUE;

    // Config group "SoftwareContainer"
    static const std::string SC_GROUP;

    // Config keys for SoftwareContainer group
    static const std::string PRELOAD_COUNT_KEY;
    static const std::string KEEP_CONTAINERS_ALIVE_KEY;
    static const std::string USE_SESSION_BUS_KEY;
    static const std::string SHUTDOWN_TIMEOUT_KEY;
    static const std::string SHARED_MOUNTS_DIR_KEY;
    static const std::string LXC_CONFIG_PATH_KEY;
    static const std::string SERVICE_MANIFEST_DIR_KEY;
    static const std::string DEFAULT_SERVICE_MANIFEST_DIR_KEY;

private:
    template<typename T>
    T getValue(const std::string &group,
               const std::string &key,
               const std::map<std::string, T> &options) const;

    template<typename T>
    T getGlibValue(const std::string &group,
                   const std::string &key) const;

    /**
     * @brief Load main config file source into member m_config
     *
     * @throws ConfigError On error
     */
    void loadConfig();

    std::unique_ptr<ConfigLoaderAbstractInterface> m_loader;
    std::unique_ptr<Glib::KeyFile> m_config;
    std::unique_ptr<ConfigDefaults> m_defaults;
    std::map<std::string, std::string> m_stringOptions;
    std::map<std::string, int> m_intOptions;
    std::map<std::string, bool> m_boolOptions;
};

} // namespace softwarecontainer
