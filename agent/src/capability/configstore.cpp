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


ConfigStore::ConfigStore(const std::string &inputPath)
{
  if (isFile(inputPath)) {
    readCapsFromFile(inputPath);
  } else if (isDirectory(inputPath)) {
    readCapsFromDir(inputPath);
  }

}

ConfigStore::~ConfigStore()
{
}

ReturnCode ConfigStore::readCapsFromFile(const std::string &filePath)
{

    json_error_t error;
    json_t *fileroot = json_load_file(filePath.c_str(), 0, &error);

    if (fileroot == nullptr) {
      return ReturnCode::FAILURE;
    }
    if (!json_is_object(fileroot)) {
      return ReturnCode::FAILURE;
    }

    json_t *capabilities = json_object_get(fileroot, "capbilities");
    if (capabilities == nullptr) {
      return ReturnCode::FAILURE;
    }
    if (!json_is_array(capabilities)) {
      return ReturnCode::FAILURE;
    }

    std::map<std::string, std::map<std::string, json_t*>> caps;

    size_t i;
    json_t *capability;

    json_array_foreach(capabilities,i,capability) {
      if (!json_is_object(capability)) {
        return ReturnCode::FAILURE;
      }

      std::string capName;
      if (!JSONParser::read(capability,"name",capName)) {
        return ReturnCode::FAILURE;
      }

      json_t *gateways = json_object_get(capability, "gateways");
      if (gateways == nullptr) {
        return ReturnCode::FAILURE;
      }
      if (!json_is_array(gateways)) {
        return ReturnCode::FAILURE;
      }
      size_t j;
      json_t *gateway;

      std::map<std::string, json_t*> gwConfs;

      json_array_foreach(gateways, j, gateway) {
        if (!json_is_object(gateway)) {
          return ReturnCode::FAILURE;
        }

        std::string gwID;
        if (!JSONParser::read(gateway,"id",gwID)) {
          return ReturnCode::FAILURE;
        }

        json_t *confs = json_object_get(gateway,"config");
        if (confs == nullptr) {
          return ReturnCode::FAILURE;
        }
        if (!json_is_array(confs)) {
          return ReturnCode::FAILURE;
        }
        gwConfs[gwID] = confs;

      }

      std::map<std::string, json_t *> oldGwConfs = m_configStore[capName];

      /* If there is already a Gateway Configuration saved with the
       * same ID, then append the configs from this file to the
       * existing Gateway in the m_configstore
       */
      for (auto it=gwConfs.begin(); it!=gwConfs.end(); ++it) {
        //std::cout << it->first << " => " << it->second << '\n';
        std::string gwID = it->first;
        json_t *val = oldGwConfs[gwID];
        if (json_array_extend(val, it->second) == -1) {
          return ReturnCode::FAILURE;
        }
      }
    }
    return ReturnCode::SUCCESS;
}

ReturnCode ConfigStore::readCapsFromDir(const std::string &dirPath)
{

  if (isDirectoryEmpty(dirPath)) {
        log_debug() << "Path to configuration files is empty";
    }

    std::vector<std::string> files;

    files = fileList(dirPath);

    if (files.empty()) {
        log_debug() << "No configuration files found";
        return ReturnCode::FAILURE;
    }

    for (unsigned int i = 0; i != files.size(); i++) {
        std::string filePath = dirPath + files[i];

        if (readCapsFromFile(filePath) != ReturnCode::SUCCESS) {
          return ReturnCode::FAILURE;
        }
    }

    return ReturnCode::SUCCESS;
}


std::map<std::string, json_t *> ConfigStore::getGatewaysByCap(const std::string capID)
{
    std::map<std::string, json_t *> gateways;
    gateways = m_configStore[capID];

    return gateways;
}


std::vector<std::string> ConfigStore::fileList(const std::string &filePath)
{
    std::vector<std::string> files;

    DIR *dirFile = opendir(filePath.c_str());
    if (dirFile == nullptr) {
        log_debug() << "Could not open directory: " << filePath << "\n";
    } else {
        struct dirent *hFile;
        while ((hFile = readdir(dirFile)) != nullptr) {
            std::string filename(hFile->d_name);

            if (is_json(filename)) {
                files.push_back(filename);
            }
        }
        closedir(dirFile);
    }

    return files;
}
bool ConfigStore::is_json(const std::string &filename) {
    size_t lastdot = filename.find_last_of(".");
    std::string prefix = filename.substr(lastdot);
    if (prefix.compare("json") != 0) {
      return false;
    }

    return true;
}
