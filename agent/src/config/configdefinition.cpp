/*
 * Copyright (C) 2016-2017 Pelagicore AB
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


#include "configdefinition.h"


/**
 * Developers guide to adding a config item:
 *
 *   * Add a definition of the string used as key
 *
 *   * If the config will be possible to set with a command line option, also add
 *     an "intial value" set to something which is possible to use for testing if
 *     the user set it, i.e. an "impossible" value.
 *
 *   * Add an entry for the config to the CONFIGS data
 */

namespace softwarecontainer {

typedef std::vector<std::tuple<std::string, std::string, ConfigType, MandatoryFlag>> ConfigItems;

const MandatoryFlag Mandatory = true;
const MandatoryFlag Optional = false;

const int ITEM_INDEX_GROUP = 0;
const int ITEM_INDEX_KEY = 1;
const int ITEM_INDEX_TYPE = 2;
const int ITEM_INDEX_MANDATORY = 3;

// Illegal values used as initial command line option values to check if user
// set them or not
const std::string ConfigDefinition::SC_CONFIG_PATH_INITIAL_VALUE = "";
const int ConfigDefinition::SHUTDOWN_TIMEOUT_INITIAL_VALUE = -2;
const std::string ConfigDefinition::SERVICE_MANIFEST_DIR_INITIAL_VALUE = "";
const std::string ConfigDefinition::DEFAULT_SERVICE_MANIFEST_DIR_INITIAL_VALUE = "";
const bool ConfigDefinition::USE_SESSION_BUS_INITIAL_VALUE = false;

// Config group "SoftwareContainer"
const std::string ConfigDefinition::SC_GROUP = "SoftwareContainer";

// Config keys for SoftwareContainer group
const std::string ConfigDefinition::SC_USE_SESSION_BUS_KEY = "session-bus";
const std::string ConfigDefinition::SC_SHUTDOWN_TIMEOUT_KEY = "shutdown-timeout";
const std::string ConfigDefinition::SC_SHARED_MOUNTS_DIR_KEY = "shared-mounts-dir";
const std::string ConfigDefinition::SC_LXC_CONFIG_PATH_KEY = "deprecated-lxc-config-path";
const std::string ConfigDefinition::SC_SERVICE_MANIFEST_DIR_KEY = "service-manifest-dir";
const std::string ConfigDefinition::SC_DEFAULT_SERVICE_MANIFEST_DIR_KEY = "default-service-manifest-dir";

#ifdef ENABLE_NETWORKGATEWAY
const std::string ConfigDefinition::SC_CREATE_BRIDGE_KEY = "create-bridge";
const std::string ConfigDefinition::SC_BRIDGE_DEVICE_KEY = "bridge-device";
const std::string ConfigDefinition::SC_BRIDGE_IP_KEY = "bridge-ip";
const std::string ConfigDefinition::SC_BRIDGE_NETADDR_KEY = "bridge-netaddr";
const std::string ConfigDefinition::SC_BRIDGE_NETMASK_KEY = "bridge-netmask";
const std::string ConfigDefinition::SC_BRIDGE_NETMASK_BITLENGTH_KEY = "bridge-netmask-bitlength";
#endif

/*
 * Used to create a mapping between group-key pairs and a type, as well as defining what configs
 * are mandatory.
 *
 * Configs that might be mandatory, i.e. that fact is decided externally by CMake, need to be added
 * with the definition and not the 'Optional' or 'Mandatory' flags directly.
 */
const ConfigItems CONFIGS
{
#ifdef ENABLE_NETWORKGATEWAY
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_CREATE_BRIDGE_KEY,
                    ConfigType::Boolean,
                    ConfigDefinition::convertDefineToFlag(
                        SC_CREATE_BRIDGE_MANDATORY_FLAG /* set by cmake */)),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_BRIDGE_DEVICE_KEY,
                    ConfigType::String,
                    ConfigDefinition::convertDefineToFlag(
                        SC_BRIDGE_DEVICE_MANDATORY_FLAG /* set by cmake */)),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_BRIDGE_IP_KEY,
                    ConfigType::String,
                    ConfigDefinition::convertDefineToFlag(
                        SC_BRIDGE_IP_MANDATORY_FLAG /* set by cmake */)),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_BRIDGE_NETMASK_BITLENGTH_KEY,
                    ConfigType::Integer,
                    ConfigDefinition::convertDefineToFlag(
                        SC_BRIDGE_NETMASK_BITLENGTH_MANDATORY_FLAG /* set by cmake */)),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_BRIDGE_NETMASK_KEY,
                    ConfigType::String,
                    ConfigDefinition::convertDefineToFlag(
                        SC_BRIDGE_NETMASK_MANDATORY_FLAG /* set by cmake */)),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_BRIDGE_NETADDR_KEY,
                    ConfigType::String,
                    ConfigDefinition::convertDefineToFlag(
                        SC_BRIDGE_NETADDR_MANDATORY_FLAG /* set by cmake */)),
#endif // ENABLE_NETWORKGATEWAY
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_USE_SESSION_BUS_KEY,
                    ConfigType::Boolean,
                    Optional),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_SHUTDOWN_TIMEOUT_KEY,
                    ConfigType::Integer,
                    Optional),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_SHARED_MOUNTS_DIR_KEY,
                    ConfigType::String,
                    Optional),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_LXC_CONFIG_PATH_KEY,
                    ConfigType::String,
                    Optional),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_SERVICE_MANIFEST_DIR_KEY,
                    ConfigType::String,
                    Optional),
    std::make_tuple(ConfigDefinition::SC_GROUP,
                    ConfigDefinition::SC_DEFAULT_SERVICE_MANIFEST_DIR_KEY,
                    ConfigType::String,
                    Optional)
};

MandatoryFlag ConfigDefinition::convertDefineToFlag(bool defined)
{
    return defined ? Mandatory : Optional;
}

MandatoryConfigs ConfigDefinition::mandatory()
{
    MandatoryConfigs mandatoryConfigs = MandatoryConfigs();

    for (auto config : CONFIGS) {
        std::string group = std::get<ITEM_INDEX_GROUP>(config);
        std::string key = std::get<ITEM_INDEX_KEY>(config);
        MandatoryFlag mandatoryFlag = std::get<ITEM_INDEX_MANDATORY>(config);

        if (Mandatory == mandatoryFlag) {
            mandatoryConfigs.push_back(UniqueKey(group, key));
        }
    }

    return mandatoryConfigs;
}

TypeMap ConfigDefinition::typeMap()
{
    TypeMap typeMap = TypeMap();

    for (auto config : CONFIGS) {
        std::string group = std::get<ITEM_INDEX_GROUP>(config);
        std::string key = std::get<ITEM_INDEX_KEY>(config);
        ConfigType configType = std::get<ITEM_INDEX_TYPE>(config);

        typeMap[UniqueKey(group, key)] = configType;
    }

    return typeMap;
}

} // namespace softwarecontainer
