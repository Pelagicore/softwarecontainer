
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


#include "config/configdefaults.h"
#include "config/config.h"


ConfigDefaults::ConfigDefaults():
    m_stringOptions(std::map<std::string, std::string>()),
    m_intOptions(std::map<std::string, int>()),
    m_boolOptions(std::map<std::string, bool>())
{
    log_debug() << "Initializing config defaults";
    /*
     * Add all values defined by the build system
     */
    m_stringOptions[Config::SHARED_MOUNTS_DIR_KEY] = SHARED_MOUNTS_DIR;
    m_stringOptions[Config::LXC_CONFIG_PATH_KEY] = LXC_CONFIG_PATH;
    m_stringOptions[Config::SERVICE_MANIFEST_DIR_KEY] = SERVICE_MANIFEST_DIR;
    m_stringOptions[Config::DEFAULT_SERVICE_MANIFEST_DIR_KEY] = DEFAULT_SERVICE_MANIFEST_DIR;

    m_intOptions[Config::PRELOAD_COUNT_KEY] = PRELOAD_COUNT;
    m_intOptions[Config::SHUTDOWN_TIMEOUT_KEY] = SHUTDOWN_TIMEOUT;

    m_boolOptions[Config::KEEP_CONTAINERS_ALIVE_KEY] = KEEP_CONTAINERS_ALIVE;
    m_boolOptions[Config::USE_SESSION_BUS_KEY] = USE_SESSION_BUS;
}

template<>
std::string ConfigDefaults::getValue(const std::string &key) const
{
    return m_stringOptions.at(key);
}

template<>
int ConfigDefaults::getValue(const std::string &key) const
{
    return m_intOptions.at(key);
}

template<>
bool ConfigDefaults::getValue(const std::string &key) const
{
    return m_boolOptions.at(key);
}
