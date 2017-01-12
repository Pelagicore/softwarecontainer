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

#include "baseconfigstore.h"

namespace softwarecontainer {

BaseConfigStore::BaseConfigStore(const std::string &inputPath)
{
    ReturnCode retval = ReturnCode::SUCCESS;

    if (inputPath.empty()) {
        //File path is empty, this is ok
        return;
    }

    if (isDirectory(inputPath)) {
        retval = readCapsFromDir(inputPath);
    } else if (isFile(inputPath)) {
        retval = readCapsFromFile(inputPath);
    } else {
        //Could not find a matching clause
        log_error() << "\"" << inputPath << "\"" << " does not exist";
        retval = ReturnCode::FAILURE;
    }

    if (isError(retval)) {
        throw ReturnCode::FAILURE;
    }
}

BaseConfigStore::~BaseConfigStore()
{
}

ReturnCode BaseConfigStore::readCapsFromDir(const std::string &dirPath)
{
    if (dirPath.compare("/") == 0) {
        log_warning() << "Searching for configuration files from root dir not allowed";
        throw ServiceManifestPathError("Path to root dir not allowed for Service Manifests");
    }
    if (isDirectoryEmpty(dirPath)) {
        log_warning() << "Path to configuration files is empty: " << dirPath;
        return ReturnCode::SUCCESS;
    }

    std::vector<std::string> files = fileList(dirPath);

    if (files.empty()) {
        log_info() << "No configuration files found: " << dirPath;
        throw ConfigStoreError("No configuration files found");
    }

    for (std::string file : files) {
        std::string filePath = dirPath + file;

        if (isError(readCapsFromFile(filePath))) {
            log_warning() << "Could not parse a file in directory: " << filePath;
            throw ServiceManifestPathError("Could not parse a file in directory");
        }
    }

    return ReturnCode::SUCCESS;
}

ReturnCode BaseConfigStore::readCapsFromFile(const std::string &filePath)
{
    if (!isJsonFile(filePath)) {
        log_debug() << "File is not a json file: " << filePath;
        throw ServiceManifestPathError("File is not a json file");
    }

    json_error_t error;
    json_t *fileroot = json_load_file(filePath.c_str(), 0, &error);

    if (nullptr == fileroot) {
        log_error() << "Could not parse Service Manifest: "
                    << filePath << ":" << error.line <<" : " << error.text;
        throw ServiceManifestParseError("Could not parse Service Manifest")
    } else if (!json_is_object(fileroot)) {
        log_error() << "Configuration is not a json object: \n"
                    << filePath << ":" << error.line <<" : " << error.text;
        throw ServiceManifestParseError("Service Manifest parse error - Configuration is not a json object");
    }

    json_t *capabilities = json_object_get(fileroot, "capabilities");
    if (nullptr == capabilities) {
        log_error() << "Could not parse Capabilities";
        throw CapabilityParseError();
        return ReturnCode::FAILURE;
    }
    if (!json_is_array(capabilities)) {
        log_error() << "Capabilities is not an array";
        //throw CapabilityParseError("Capabilities is not an array");
        return ReturnCode::FAILURE;
    }

    // Can't use json_decref on fileroot, it removes objects too early
    try {
        parseCapabilities(capabilities);
    } catch(CapabilityParseError &error) {
        throw;
    }
}

ReturnCode BaseConfigStore::parseCapabilities(json_t *capabilities)
{
    size_t i;
    json_t *capability;

    log_debug() << "Size of capabilities is " << std::to_string(json_array_size(capabilities));

    json_array_foreach(capabilities, i, capability) {
        if (!json_is_object(capability)) {
            log_error() << "Capability is not a json object";
            throw CapabilityParseError("Capability parse error - Capability is not a json object");
        }

        std::string capName;
        if (!JSONParser::read(capability, "name", capName)) {
            log_error() << "Could not read Capability name";
            throw CapabilityParseError("Capability parse error - Could not read Capability name");
        }

        json_t *gateways = json_object_get(capability, "gateways");
        if (nullptr == gateways) {
            log_error() << "Could not read Gateways";
            throw CapabilityParseError("Capability parse error - Could not read Gateways");
        }
        if (!json_is_array(gateways)) {
            log_error() << "Gateways is not an array";
            throw CapabilityParseError("Capability parse error - Gateways is not an array");
        }

        log_debug() << "Found capability \"" << capName << "\", parsing gateways...";

        // TODO (ethenor): Is this correct?
        try {
            parseGatewayConfigs(capName, gateways);
        } catch(CapabilityParseError &error) {
            throw;
        }
    }
    return ReturnCode::SUCCESS;
}

ReturnCode BaseConfigStore::parseGatewayConfigs(std::string capName, json_t *gateways)
{
    if (m_capMap.count(capName) > 0) {
        log_debug() << "Capability " << capName << " already loaded.";
        return ReturnCode::SUCCESS;
    }

    size_t i;
    json_t *gateway;

    GatewayConfiguration gwConf;
    json_array_foreach(gateways, i, gateway) {
        if (!json_is_object(gateway)) {
            log_error() << "Gateway is not a json object";
            throw CapabilityParseError("Capability parse error - Gateways is not json object");
        }

        std::string gwID;
        if (!JSONParser::read(gateway, "id", gwID)) {
            log_error() << "Could not read Gateway ID";
            throw CapabilityParseError("Capability parse error - Could not read Gateway ID");
        }

        json_t *confs = json_object_get(gateway, "config");
        if (nullptr == confs) {
            log_error() << "Could not read Gateway Config";
            throw CapabilityParseError("Capability parse error - Could not read Gateway config");
        }
        if (!json_is_array(confs)) {
            log_error() << "Gateway Config is not an array";
            throw CapabilityParseError("Capability parse error - Gateways config is not an array");
        }
        gwConf.append(gwID, confs);
    }
    m_capMap[capName] = gwConf;

    return ReturnCode::SUCCESS;
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
