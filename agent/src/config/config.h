
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


/*
 * TODO:
 *      * Propose we have this inherit a general SoftwareContainer exception instead
 *      * And also add some error types that can indicate what went wrong
 */
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
     * If no explicit configs are to be used, this Constructor can be called
     * by passing only the 'loader'. Otherwise all parameters needs to be passed.
     *
     * @param loader An interface to a loader which provides the underlying
     *               config to this class
     * @param stringOptions A string:string map with config keys and corresponding values
     * @param intOptions A string:int map with config keys and corresponding values
     * @param boolOptions A string:bool map with config keys and corresponding values
     */
    Config(std::unique_ptr<ConfigLoaderAbstractInterface> loader,
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

    // Config group "SoftwareContainer"
    static const std::string SC_GROUP;

    // Config keys for SoftwareContainer group
    static const std::string PRELOAD_COUNT;
    static const std::string KEEP_ALIVE;
    static const std::string USE_SESSION_BUS;
    static const std::string SHUTDOWN_TIMEOUT;
    static const std::string SHARED_MOUNTS_DIR;
    static const std::string LXC_CONFIG_PATH;
    static const std::string SERVICE_MANIFEST_DIR;
    static const std::string DEFAULT_SERVICE_MANIFEST_DIR;

private:
    std::unique_ptr<Glib::KeyFile> m_config;
    std::map<std::string, std::string> m_stringOptions;
    std::map<std::string, int> m_intOptions;
    std::map<std::string, bool> m_boolOptions;
};
