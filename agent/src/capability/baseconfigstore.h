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

/**
 * @file baseconfigstore.h
 * @brief Contains the softwarecontainer::BaseConfigStore class
 */

#pragma once

#include "softwarecontainer-common.h"
#include "gatewayconfig.h"
#include "configstoreerror.h"
#include "servicemanifestloader.h"

#include <jsonparser.h>


namespace softwarecontainer {

class BaseConfigStore
{
    LOG_DECLARE_CLASS_CONTEXT("BCS", "BaseConfigStore");

public:
    /**
     * @brief Creates a new BaseConfigStore object and searches for Service Manifests
     * (of file type json) in the input path, and stores the Capabilities'
     * Gateway configurations.
     *
     * @throws ServiceManifestPathError if the path to the json file(s) is incorrectly formatted
     * or if the path is not allowed
     * @throws ServiceManifestParseError if parsing of the json fails
     * @throws CapabilityParseError if parsing of one or more Capabilities or Gateway
     *         objects is unsuccessful
     */
    BaseConfigStore(std::unique_ptr<ServiceManifestLoader> loader);

protected:
    std::map<std::string, GatewayConfiguration> m_capMap;

private:
    /**
     * @brief Parse a JSON object, which should be a JSON array, of capabilities
     * and saves them in the internal storage.
     * If a capability with the same name already exists in the storage
     * the gateways object will be appended to the previously stored gateway list.
     *
     * @throws CapabilityParseError if parsing of the service manifest is unsuccessful
     */
    void parseServiceManifest(json_t *serviceManifest);

    /**
     * @brief Parse a JSON object, which should be a JSON array, of gateway configurations
     * and saves them in the internal storage.
     *
     * @throws CapabilityParseError if parsing of gateway configurations is unsuccessful
     */
    void parseGatewayConfigs(std::string capName, json_t *gateways);
};

} // namespace softwarecontainer
