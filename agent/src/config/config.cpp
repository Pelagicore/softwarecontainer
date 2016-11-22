
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


// Config group "SoftwareContainer"
const std::string Config::SC_GROUP = "SoftwareContainer";

// Config keys for SoftwareContainer group
const std::string Config::PRELOAD_COUNT = "preload-count";
const std::string Config::KEEP_ALIVE = "keep-containers-alive";
const std::string Config::USE_SESSION_BUS = "session-bus";
const std::string Config::SHUTDOWN_TIMEOUT = "shutdown-timeout";
const std::string Config::SHARED_MOUNTS_DIR = "shared-mounts-dir";
const std::string Config::LXC_CONFIG_PATH = "deprecated-lxc-config-path";
const std::string Config::SERVICE_MANIFEST_DIR = "service-manifest-dir";
const std::string Config::DEFAULT_SERVICE_MANIFEST_DIR = "default-service-manifest-dir";


Config::Config(std::unique_ptr<ConfigLoaderAbstractInterface> loader,
               const std::map<std::string, std::string> &stringOptions,
               const std::map<std::string, int> &intOptions,
               const std::map<std::string, bool> &boolOptions) :
    m_stringOptions(stringOptions),
    m_intOptions(intOptions),
    m_boolOptions(boolOptions)
{
    try {
        m_config = loader->loadConfig();
    } catch (Glib::Error &error) {
        log_error() << "Could not load SoftwareContainer config: \"" << error.what() << "\"";
        throw softwarecontainer::ConfigError();
    }
}

std::string Config::getStringValue(const std::string &group, const std::string &key) const
{
    // Check if this key has been explicitly set and if so, return that value
    std::map<std::string, std::string>::const_iterator position = m_stringOptions.find(key);
    if (position != m_stringOptions.end()) {
        // If the end wasn't reached, the value is where the iterator is now
        return position->second;
    }

    // Look for value in the config
    std::string value = "";
    try {
        value = m_config->get_string(Glib::ustring(group), Glib::ustring(key));
    } catch (Glib::Error &error) {
        log_error() << "Could not parse string value: \"" << error.what() << "\"";
        throw softwarecontainer::ConfigError();
    }

    return value;
}

int Config::getIntegerValue(const std::string &group, const std::string &key) const
{
    // Check if this key has been explicitly set and if so, return that value
    std::map<std::string, int>::const_iterator position = m_intOptions.find(key);
    if (position != m_intOptions.end()) {
        // If the end wasn't reached, the value is where the iterator is now
        return position->second;
    }

    // Look for value in the config
    int value = 0;
    try {
        value = m_config->get_integer(Glib::ustring(group), Glib::ustring(key));
    } catch (Glib::Error &error) {
        log_error() << "Could not parse integer value: \"" << error.what() << "\"";
        throw softwarecontainer::ConfigError();
    }

    return value;
}

bool Config::getBooleanValue(const std::string &group, const std::string &key) const
{
    // Check if this key has been explicitly set and if so, return that value
    std::map<std::string, bool>::const_iterator position = m_boolOptions.find(key);
    if (position != m_boolOptions.end()) {
        // If the end wasn't reached, the value is where the iterator is now
        return position->second;
    }

    // Look for value in the config
    bool value = false;
    try {
        value = m_config->get_boolean(Glib::ustring(group), Glib::ustring(key));
    } catch (Glib::Error &error) {
        log_error() << "Could not parse boolean value: \"" << error.what() << "\"";
        throw softwarecontainer::ConfigError();
    }

    return value;
}
