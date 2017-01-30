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

#pragma once

#include "softwarecontainer-common.h"
#include "configtypes.h"


namespace softwarecontainer {

/*
 * Used for the group-key combination that makes configs unique
 */
typedef std::pair<std::string, std::string> UniqueKey;

/*
 * Defines configs, as group and key, to be considered mandatory by Config
 *
 * A call to ConfigDefinition::mandatory will return an object of this type which is
 * intended to be used when creating a Config object.
 *
 * Normally, code external to ConfigDefinition should not create objects of type
 * MandatoryConfigs, but it is available for e.g test code that needs to explicitly
 * control the content.
 */
typedef std::vector<UniqueKey> MandatoryConfigs;

/*
 * Maps a dependee config to a list of dependencies. This is passed to Config
 * on creation.
 */
typedef std::map<UniqueKey, std::vector<UniqueKey>> ConfigDependencies;

/*
 * Maps a config to a ConfigType. This is passed to Config on creation.
 */
typedef std::map<UniqueKey, ConfigType> TypeMap;

/*
 * Used internally by ConfigDefinition to mark configs to be mandatory or optional
 */
typedef bool MandatoryFlag;


class ConfigDefinition
{
public:
    // Illegal values used as initial command line option values to check if user
    // set them or not
    static const std::string SC_CONFIG_PATH_INITIAL_VALUE;
    static const int SHUTDOWN_TIMEOUT_INITIAL_VALUE;
    static const std::string SERVICE_MANIFEST_DIR_INITIAL_VALUE;
    static const std::string DEFAULT_SERVICE_MANIFEST_DIR_INITIAL_VALUE;
    static const bool USE_SESSION_BUS_INITIAL_VALUE;

    // Config group "SoftwareContainer"
    static const std::string SC_GROUP;

    // Config keys for SoftwareContainer group
    static const std::string SC_USE_SESSION_BUS_KEY;
    static const std::string SC_SHUTDOWN_TIMEOUT_KEY;
    static const std::string SC_SHARED_MOUNTS_DIR_KEY;
    static const std::string SC_LXC_CONFIG_PATH_KEY;
    static const std::string SC_SERVICE_MANIFEST_DIR_KEY;
    static const std::string SC_DEFAULT_SERVICE_MANIFEST_DIR_KEY;

#ifdef ENABLE_NETWORKGATEWAY
    static const std::string SC_CREATE_BRIDGE_KEY;
    static const std::string SC_BRIDGE_DEVICE_KEY;
    static const std::string SC_BRIDGE_IP_KEY;
    static const std::string SC_BRIDGE_NETADDR_KEY;
    static const std::string SC_BRIDGE_NETMASK_KEY;
    static const std::string SC_BRIDGE_NETMASK_BITS_KEY;
#endif

    static MandatoryConfigs mandatory();
    static ConfigDependencies dependencies();
    static TypeMap typeMap();
    static MandatoryFlag convertDefineToFlag(bool definition);
};

} // namespace softwarecontainer
