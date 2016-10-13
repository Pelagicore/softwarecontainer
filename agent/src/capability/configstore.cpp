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

ConfigStore::~ConfigStore()
{
}

std::vector<GatewayConfiguration> softwarecontainer::capFromFile(std::string capID,
								 std::string &filePath)
{
    std::vector<GatewayConfiguration> GWconfs;

    if (!configFilePath.empty()) {
        GWconfs = getGatewaysByCap(capID, filePath);
    }

    return GWconfs;
}
/*
std::vector<Capability> ConfigParser::capsFromConfigs(std::string &configDirPath)
{
    if (configDirPath.empty()) {
        log_debug() << "Path to configuration files is empty";
    }

    if (!hasEnding(configDirPath, "/")) {
        configDirPath += "/";
    }

    std::vector<Capability> caps;
    std::vector<std::string> configFiles;

    configFiles = configFileList(configDirPath);

    if (configFiles.empty()) {
        log_debug() << "No configuration files found";
    }

    for (unsigned int i = 0; i != configFiles.size(); i++) {
        std::string configFilePath = configDirPath + configFiles[i];

        std::vector<Capability> configCaps = capsFromConfig(configFilePath);

        caps.insert(caps.end(), configCaps.begin(), configCaps.end());
    }

    return caps;
}
*/

std::vector<GatewayConfiguration> softwarecontainer::getGatewaysByCap(std::string capID,
								      const std::string &filePath) {
    std::vector<GatewayConfiguration> gateways;

    JSonElement root = JSonElement::fromFile(filePath);

    if (root.isValid()) {

        auto arrayElement = root.getField("capabilities");

        if (arrayElement.isArray()) {
            std::vector<JSonElement> children;
            arrayElement.readChildren(children);
            for (auto& child : children) {

                std::string capName;
                child.read("name", capName);

		// TODO (thenor): Only push gateways for capID
		if (capName == capID) {
		    auto gatewaysElement = child.getField("gateways");
		    if (arrayElement.isArray()) {

		        std::string strError;
			if (!parseGatewayData(gatewaysElement, gateways, &strError)) {
			  log_debug() << "Error in " << pathString << ": " << strError;
			  gateways.clear();
			}
			else {
			  GatewayConfiguration gw = GatewayConfiguration(gatewayName, gwConf);
			  gateways.push_back(gw);
			}
		    }
		} else {
		  log_debug() << "Capability " << capName << " does not match " << capID;
		}
            }
        } else {
            log_debug() << "Error in " << pathString << " : capabilities must be an array";
        }
    } else {
        log_error() << "Error in JSON file : " << pathString;
    }

    return gateways;
}


bool softwarecontainer::parseGatewayData(const JSonElement &gatewayData,
					 std::vector<GatewayConfiguration> &gateways,
					 std::string *error)
{
    bool success = false;

    std::vector<JSonElement> gatewayElements;
    gatewayData.readChildren(gatewayElements);

    for(auto& gatewayElement: gatewayElements) {

        std::string gwId;
        gatewayElement.read("id", gwId);

        Gateway gw = Gateway(gwId, gatewayElement.getField("config").dump());
        gateways.push_back(gw);

        success = true;
    }

    return success;
}

std::vector<std::string> softwarecontainer::configFileList(const std::string &configPath)
{
    std::vector<std::string> configFiles;

    DIR *dirFile = opendir(configPath.c_str());
    if (dirFile == nullptr) {
        log_debug() << "Could not open directory: " << configPath << "\n";
    } else {
        struct dirent *hFile;
        while ((hFile = readdir(dirFile)) != nullptr) {
            std::string filename(hFile->d_name);

            if (pelagicore::has_suffix(filename, ".json"))
                configFiles.push_back(filename);
        }
        closedir(dirFile);
    }

    return configFiles;
}

bool softwarecontainer::hasEnding(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}
