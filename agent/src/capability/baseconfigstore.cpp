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

BaseConfigStore::BaseConfigStore(const std::string &inputPath)
{
    if (inputPath.empty()) {
        //File path is empty, this is ok
        return;
    }

    if (isDirectory(inputPath)) {
        readCapsFromDir(inputPath);
    } else if (isFile(inputPath)) {
        readCapsFromFile(inputPath);
    } else {
        std::string errorMessage = "Path to service manifest does not exist: \"" + inputPath + "\"";
        log_error() << errorMessage;
        throw ServiceManifestPathError(errorMessage);
    }
}

BaseConfigStore::~BaseConfigStore()
{
}

void BaseConfigStore::readCapsFromDir(const std::string &dirPath)
{
    std::string errorMessage;

    if (dirPath.compare("/") == 0) {
        errorMessage = "Searching for configuration files from root dir not allowed";
        log_error() << errorMessage;
        throw ServiceManifestPathError(errorMessage);
    }
    if (isDirectoryEmpty(dirPath)) {
        log_warning() << "Path to configuration files is empty: " << dirPath;
        return;
    }

    std::vector<std::string> files = fileList(dirPath);

    if (files.empty()) {
        log_info() << "No configuration files found: " << dirPath;
        return;
    }

    for (std::string file : files) {
        std::string filePath = dirPath + file;

        readCapsFromFile(filePath);
    }
}

void BaseConfigStore::readCapsFromFile(const std::string &filePath)
{
    std::string errorMessage;

    if (!isJsonFile(filePath)) {
        errorMessage = "File is not a json file: " + filePath;
        log_debug() << errorMessage;
        throw ServiceManifestParseError(errorMessage);
    }

    json_error_t error;
    json_t *fileroot = json_load_file(filePath.c_str(), 0, &error);

    if (nullptr == fileroot) {
        errorMessage = "Could not parse Service Manifest: "
            + filePath + ":" + std::to_string(error.line) + " : " + std::string(error.text);
        log_error() << errorMessage;
        throw ServiceManifestParseError(errorMessage);
    } else if (!json_is_object(fileroot)) {
        errorMessage = "Configuration is not a json object: "
            + filePath + ":" + std::to_string(error.line) + " : " + std::string(error.text);
        log_error() << errorMessage;
        throw ServiceManifestParseError(errorMessage);
    }

    json_t *capabilities = json_object_get(fileroot, "capabilities");
    if (nullptr == capabilities) {
        errorMessage = "Could not parse Capabilities in file: " + filePath;
        log_error() << errorMessage;
        throw CapabilityParseError(errorMessage);
    }
    if (!json_is_array(capabilities)) {
        errorMessage = "Capabilities is not an array, in file: " + filePath;
        log_error() << errorMessage;
        throw CapabilityParseError(errorMessage);
    }

    // Can't use json_decref on fileroot, it removes objects too early
    parseCapabilities(capabilities);
}

void BaseConfigStore::parseCapabilities(json_t *capabilities)
{
    size_t i;
    json_t *capability;
    std::string errorMessage;

    log_debug() << "Size of capabilities is " << std::to_string(json_array_size(capabilities));

    json_array_foreach(capabilities, i, capability) {
        if (!json_is_object(capability)) {
            errorMessage = "Capability is not a json object";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }

        std::string capName;
        if (!JSONParser::read(capability, "name", capName)) {
            errorMessage = "Could not read Capability name";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }

        json_t *gateways = json_object_get(capability, "gateways");
        if (nullptr == gateways) {
            errorMessage = "Could not read Gateways";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }
        if (!json_is_array(gateways)) {
            errorMessage = "Gateways is not an array";
            log_error() << errorMessage;
            throw CapabilityParseError(errorMessage);
        }

        log_debug() << "Found capability \"" << capName << "\", parsing gateways...";

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
            errorMessage = "The \"gateway\" key in the Service Manifest is not a json object";
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

std::vector<std::string> BaseConfigStore::fileList(const std::string &dirPath)
{
    std::vector<std::string> files;

    DIR *dirFile = opendir(dirPath.c_str());
    if (nullptr == dirFile) {
        log_error() << "Could not open directory: " << dirPath;
        return files;
    }

    struct dirent *hFile;
    while ((hFile = readdir(dirFile)) != nullptr) {
        std::string filename(hFile->d_name);
        if (isJsonFile(filename)) {
            files.push_back(filename);
        }
    }
    closedir(dirFile);

    return files;
}

bool BaseConfigStore::isJsonFile(const std::string &filename) {

    if (filename.size() <= 5) {
        // Even if the file name is ".json" we will not classify this as a json file
        return false;
    }
    return filename.substr(filename.size()-5) == ".json";
}

} // namespace softwarecontainer
