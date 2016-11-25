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
        return ReturnCode::FAILURE;
    }
    if (isDirectoryEmpty(dirPath)) {
        log_warning() << "Path to configuration files is empty: " << dirPath;
        return ReturnCode::SUCCESS;
    }

    std::vector<std::string> files = fileList(dirPath);

    if (files.empty()) {
        log_info() << "No configuration files found: " << dirPath;
        return ReturnCode::SUCCESS;
    }

    for (std::string file : files) {
        std::string filePath = dirPath + file;

        if (readCapsFromFile(filePath) != ReturnCode::SUCCESS) {
            log_warning() << "Could not parse a file in directory: " << filePath;
        }
    }

    return ReturnCode::SUCCESS;
}

ReturnCode BaseConfigStore::readCapsFromFile(const std::string &filePath)
{
    if (!isJsonFile(filePath)) {
        log_debug() << "File is not a json file: " << filePath;
        return ReturnCode::FAILURE;
    }

    json_error_t error;
    json_t *fileroot = json_load_file(filePath.c_str(), 0, &error);

    if (nullptr == fileroot) {
        log_error() << "Could not parse Service Manifest: "
                    << filePath << ":" << error.line <<" : " << error.text;
        return ReturnCode::FAILURE;
    } else if (!json_is_object(fileroot)) {
        log_error() << "Configuration is not a json object: \n"
                    << filePath << ":" << error.line <<" : " << error.text;
        return ReturnCode::FAILURE;
    }

    json_t *capabilities = json_object_get(fileroot, "capabilities");
    if (nullptr == capabilities) {
        log_error() << "Could not parse Capabilities";
        return ReturnCode::FAILURE;
    }
    if (!json_is_array(capabilities)) {
        log_error() << "Capabilities is not an array";
        return ReturnCode::FAILURE;
    }

    return parseCapabilities(capabilities);
}

ReturnCode BaseConfigStore::parseCapabilities(json_t *capabilities)
{
    size_t i;
    json_t *capability;

    log_debug() << "Size of capabilities is " << std::to_string(json_array_size(capabilities));

    json_array_foreach(capabilities, i, capability) {
        if (!json_is_object(capability)) {
            log_error() << "Capability is not a json object";
            return ReturnCode::FAILURE;
        }

        std::string capName;
        if (!JSONParser::read(capability, "name", capName)) {
            log_error() << "Could not read Capability name";
            return ReturnCode::FAILURE;
        }

        json_t *gateways = json_object_get(capability, "gateways");
        if (nullptr == gateways) {
            log_error() << "Could not read Gateways";
            return ReturnCode::FAILURE;
        }
        if (!json_is_array(gateways)) {
            log_error() << "Gateways is not an array";
            return ReturnCode::FAILURE;
        }

        log_debug() << "Found capability \"" << capName << "\", parsing gateways...";
        parseGatewayConfigs(capName, gateways);
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
            return ReturnCode::FAILURE;
        }

        std::string gwID;
        if (!JSONParser::read(gateway, "id", gwID)) {
            log_error() << "Could not read Gateway ID";
            return ReturnCode::FAILURE;
        }

        json_t *confs = json_object_get(gateway, "config");
        if (nullptr == confs) {
            log_error() << "Could not read Gateway Config";
            return ReturnCode::FAILURE;
        }
        if (!json_is_array(confs)) {
            log_error() << "Gateway Config is not an array";
            return ReturnCode::FAILURE;
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
    size_t lastdot = filename.find_last_of(".");
    std::string prefix = filename.substr(lastdot);
    if (prefix.compare(".json") != 0) {
        log_debug() << "File does not have json as prefix (" << prefix <<")";
        return false;
    }
    return true;
}
