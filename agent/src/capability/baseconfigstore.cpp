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

#include "baseconfigstore.h"

namespace softwarecontainer {

BaseConfigStore::BaseConfigStore(std::unique_ptr<ServiceManifestLoader> loader)
{
    std::vector<json_t *> content = loader->loadContent();

    for (json_t *serviceManifest : content) {
        parseServiceManifest(serviceManifest);
    }
}

void BaseConfigStore::parseServiceManifest(json_t *serviceManifest)
{
    size_t i;
    json_t *capability;
    std::string errorMessage;

    json_t *capabilities = json_object_get(serviceManifest, "capabilities");
    if (nullptr == capabilities) {
        errorMessage = "Could not parse the \"capability\" object";
        log_error() << errorMessage;
        throw CapabilityParseError(errorMessage);
    }
    if (!json_is_array(capabilities)) {
        errorMessage = "The \"capability\" key does not point to an array";
        log_error() << errorMessage;
        throw CapabilityParseError(errorMessage);
    }

    log_debug() << "Size of capabilities is " << std::to_string(json_array_size(capabilities));

    json_array_foreach(capabilities, i, capability) {
        if (!json_is_object(capability)) {
            errorMessage = "A \"capability\" in the Service Manifest is not a json object";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }

        std::string capName;
        if (!JSONParser::read(capability, "name", capName)) {
            errorMessage = "Could not read the name of the \"capability\" object";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }

        json_t *gateways = json_object_get(capability, "gateways");
        if (nullptr == gateways) {
            errorMessage = "Could not read the \"gateway\" objects in \"" + capName + "\"";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }
        if (!json_is_array(gateways)) {
            errorMessage = "The \"gateway\" object is not an array";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }

        log_debug() << "Found capability \"" << capName << "\"";

        parseGatewayConfigs(capName, gateways);
    }
}

void BaseConfigStore::parseGatewayConfigs(std::string capName, json_t *gateways)
{
    if (m_capMap.count(capName) > 0) {
        log_debug() << "Capability " << capName << " already loaded.";
        return;
    }

    size_t i;
    json_t *gateway;
    std::string errorMessage;

    GatewayConfiguration gwConf;
    json_array_foreach(gateways, i, gateway) {
        if (!json_is_object(gateway)) {
            errorMessage = "A \"gateway\" in the Service Manifest is not a json object";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }

        std::string gwID;
        if (!JSONParser::read(gateway, "id", gwID)) {
            errorMessage = "Could not read the ID of the \"gateway\""
                " object in the Service Manifest";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }

        json_t *confs = json_object_get(gateway, "config");
        if (nullptr == confs) {
            errorMessage = "Could not read the \"gateway\" object's configuration element "
                "(" + gwID+ ")";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }
        if (!json_is_array(confs)) {
            errorMessage = "The \"gateway\" object's configuration is not an array (" + gwID+ ")";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }
        gwConf.append(gwID, confs);
    }
    m_capMap[capName] = gwConf;
}

} // namespace softwarecontainer
