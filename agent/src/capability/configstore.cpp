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

#include "configstore.h"


ConfigStore::ConfigStore()
{
}

ConfigStore::ConfigStore(const std::string &inputPath)
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

ConfigStore::~ConfigStore()
{
}

ReturnCode ConfigStore::readCapsFromDir(const std::string &dirPath)
{
    if (dirPath.compare("/") == 0) {
        log_warning() << "Searching for configuration files from root dir not allowed";
        return ReturnCode::FAILURE;
    }
    if (isDirectoryEmpty(dirPath)) {
        log_warning() << "Path to configuration files is empty: " << dirPath;
        return ReturnCode::FAILURE;
    }

    std::vector<std::string> files = fileList(dirPath);

    if (files.empty()) {
        log_info() << "No configuration files found: " << dirPath;
        return ReturnCode::SUCCESS;
    }

    // log_debug() << "The list of files:";
    // for (std::string file : files) {
    //     log_debug() << "File: " << file;
    // }

    for (std::string file : files) {
        std::string filePath = dirPath + file;

        if (readCapsFromFile(filePath) != ReturnCode::SUCCESS) {
            log_warning() << "Could not parse a file in directory: " << filePath;
        }
    }

    return ReturnCode::SUCCESS;
}

ReturnCode ConfigStore::readCapsFromFile(const std::string &filePath)
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


GatewayConfiguration ConfigStore::getGatewayConfigs(const std::string capID)
{
    if (m_configStore.count(capID) == 0) {
        log_warning() << "Couldn't find " << capID << " in ConfigStore";
        log_debug() << "ConfigStore contains "
                    << std::to_string(m_configStore.size()) << " element(s)";
        return GatewayConfiguration();
    }
    return m_configStore[capID];
}


ReturnCode ConfigStore::parseCapabilities(json_t *capabilities)
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

ReturnCode ConfigStore::parseGatewayConfigs(std::string capName, json_t *gateways)
{
    size_t i;
    json_t *gateway;

    GatewayConfiguration gwConfs;

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
        gwConfs[gwID] = confs;

    }

    GatewayConfiguration oldGwConfs = m_configStore[capName];

    /* If there is already a Gateway Configuration saved with the
     * same ID, then append the configs from this file to the
     * existing Gateway in the m_configstore
     */
    for (auto const &it : gwConfs) {
        std::string gwID = it.first;
        if (oldGwConfs.count(gwID) == 1) {
            json_t *val = oldGwConfs[gwID];
            log_debug() << "Appending gateway configuration to " << gwID;
            if (json_array_extend(val, it.second) == -1) {
                log_error() << "Could not add Gateway Config to json array: " << gwID;
                return ReturnCode::FAILURE;
            }
        } else {
            log_debug() << "Adding gateway configuration for new element " << gwID;
            oldGwConfs[gwID] = it.second;
        }
    }
    log_debug() << "Adding/Updating gateway configuration for capability \"" << capName << "\"";
    m_configStore[capName] = oldGwConfs;

    return ReturnCode::SUCCESS;
}

std::vector<std::string> ConfigStore::fileList(const std::string &dirPath)
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
bool ConfigStore::isJsonFile(const std::string &filename) {
    size_t lastdot = filename.find_last_of(".");
    std::string prefix = filename.substr(lastdot);
    if (prefix.compare(".json") != 0) {
        log_debug() << "File does not have json as prefix (" << prefix <<")";
        return false;
    }
    return true;
}
