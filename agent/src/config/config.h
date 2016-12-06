
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


namespace softwarecontainer {

class ConfigError : public std::exception
{
    virtual const char *what() const throw()
    {
        return "SoftwareContainer configuration error.";
    }
};
}


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
     * @throws softwarecontainer::ConfigError On error
     */
    Config(std::unique_ptr<ConfigLoaderAbstractInterface> loader,
           std::unique_ptr<ConfigDefaults> defaults,
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

    std::unique_ptr<Glib::KeyFile> m_config;
    std::unique_ptr<ConfigDefaults> m_defaults;
    std::map<std::string, std::string> m_stringOptions;
    std::map<std::string, int> m_intOptions;
    std::map<std::string, bool> m_boolOptions;
};
