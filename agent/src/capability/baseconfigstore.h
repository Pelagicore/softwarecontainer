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
 * @file configstore.h
 * @brief Contains the softwarecontainer::BaseConfigStore class
 */

#pragma once
#include "softwarecontainer-common.h"
#include "gatewayconfig.h"

#include <jsonparser.h>

namespace softwarecontainer {

class BaseConfigStore
{
    LOG_DECLARE_CLASS_CONTEXT("BCS", "BaseConfigStore");

public:
    /*
     * @brief Creates a new BaseConfigStore object and searches for Service Manifests
     * (of file type json) in the input path, and stores the Capabilities'
     * Gateway configurations.
     *
     * @throws ReturnCode::FAILURE if parsing of the json file(s) fails
     */
    BaseConfigStore(const std::string &filePath);

    virtual ~BaseConfigStore();

protected:
    std::map<std::string, GatewayConfiguration> m_capMap;

private:

    /**
     * @brief Reads a directory and finds all json Service Manifest files
     * and adds the capabilities to the internal storage.
     * If a capability with the same name already exists in the storage
     * the gateways will be appended to the previously stored gatway list.
     *
     * @return ReturnCode::SUCCESS if successful
     * @return ReturnCode::FAILURE if parsing of directory or files is unsuccessful
     *
     */
    virtual ReturnCode readCapsFromDir(const std::string &dirPath);

    /**
     * @brief Reads a Service Manifest file, of type json, and adds the
     * capabilities to the internal storage.
     * If a capability with the same name already exists in the storage
     * the gateways will be appended to the previously stored gatway list.
     *
     * @return ReturnCode::SUCCESS if successful
     * @return ReturnCode::FAILURE if parsing of file is unsuccessful
     *
     */
    virtual ReturnCode readCapsFromFile(const std::string &filePath);

    /**
     * @brief Parse a JSON object, which should be a JSON array, of capabilities
     * and saves them in the internal storage.
     *
     * @return ReturnCode::SUCCESS if successful
     * @return ReturnCode::FAILURE if parsing of files is unsuccessful
     */
    ReturnCode parseCapabilities(json_t *capabilities);

    /**
     * @brief Parse a JSON object, which should be a JSON array, of gateway configurations
     * and saves them in the internal storage.
     *
     * @return ReturnCode::SUCCESS if successful
     * @return ReturnCode::FAILURE if parsing of gateway configurations is unsuccessful
     */
    ReturnCode parseGatewayConfigs(std::string capName, json_t *gateways);

    /**
     * @brief Iterates through the files in a directory and finds all json files.
     *
     * @param a string representation of the directory path
     *
     * @return a vector of all json files present in the directory, an empty list if no
     * json files can be found in the directory
     */
    std::vector<std::string> fileList(const std::string &filePath);

    /**
     * @brief Checks if a file name ends in ".json"
     *
     * @return true/false
     */
    bool isJsonFile(const std::string &filename);

};

} // namespace softwarecontainer
