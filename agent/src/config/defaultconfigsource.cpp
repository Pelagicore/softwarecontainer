
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


#include "softwarecontainer-common.h"
#include "config.h"
#include "configdefinition.h"
#include "defaultconfigsource.h"


namespace softwarecontainer {

DefaultConfigSource::DefaultConfigSource():
    m_stringConfigs(std::vector<StringConfig>()),
    m_intConfigs(std::vector<IntConfig>()),
    m_boolConfigs(std::vector<BoolConfig>())
{
    log_debug() << "Initializing config defaults source";

    /*
    * Add all values defined by the build system
    */

    StringConfig stringConfig = StringConfig(ConfigDefinition::SC_GROUP,
                                             ConfigDefinition::SC_SHARED_MOUNTS_DIR_KEY,
                                             SC_SHARED_MOUNTS_DIR);
    stringConfig.setSource(ConfigSourceType::Default);
    m_stringConfigs.push_back(stringConfig);

    stringConfig = StringConfig(ConfigDefinition::SC_GROUP,
                                ConfigDefinition::SC_LXC_CONFIG_PATH_KEY,
                                SC_LXC_CONFIG_PATH);
    stringConfig.setSource(ConfigSourceType::Default);
    m_stringConfigs.push_back(stringConfig);

    stringConfig = StringConfig(ConfigDefinition::SC_GROUP,
                                ConfigDefinition::SC_SERVICE_MANIFEST_DIR_KEY,
                                SC_SERVICE_MANIFEST_DIR);
    stringConfig.setSource(ConfigSourceType::Default);
    m_stringConfigs.push_back(stringConfig);

    stringConfig = StringConfig(ConfigDefinition::SC_GROUP,
                                ConfigDefinition::SC_DEFAULT_SERVICE_MANIFEST_DIR_KEY,
                                SC_DEFAULT_SERVICE_MANIFEST_DIR);
    stringConfig.setSource(ConfigSourceType::Default);
    m_stringConfigs.push_back(stringConfig);

    IntConfig intConfig = IntConfig(ConfigDefinition::SC_GROUP,
                                    ConfigDefinition::SC_SHUTDOWN_TIMEOUT_KEY,
                                    SC_SHUTDOWN_TIMEOUT);
    intConfig.setSource(ConfigSourceType::Default);
    m_intConfigs.push_back(intConfig);

    BoolConfig boolConfig = BoolConfig(ConfigDefinition::SC_GROUP,
                            ConfigDefinition::SC_USE_SESSION_BUS_KEY,
                            SC_USE_SESSION_BUS);
    boolConfig.setSource(ConfigSourceType::Default);
    m_boolConfigs.push_back(boolConfig);
}

std::vector<StringConfig> DefaultConfigSource::stringConfigs()
{
    return m_stringConfigs;
}

std::vector<IntConfig> DefaultConfigSource::intConfigs()
{
    return m_intConfigs;
}

std::vector<BoolConfig> DefaultConfigSource::boolConfigs()
{
    return m_boolConfigs;
}

} // namespace softwarecontainer
