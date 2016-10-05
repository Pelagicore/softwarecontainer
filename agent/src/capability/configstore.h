
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

/**
 * @file configstore.h
 * @brief Contains the softwarecontainer::ConfigStore class
 */

// Includes copied from Agent
/*
#include <glib-unix.h>
#include <glibmm.h>

#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>
#include <getopt.h>
*/

#include "softwarecontainer-common.h"

#include <jsonparser.h>

namespace softwarecontainer {

  class ConfigStore
  {
    LOG_DECLARE_CLASS_CONTEXT("CS", "ConfigStore");

  public:
    ConfigStore(const std::string &filePath);
    ~ConfigStore();

    // Fetch by capability ID
    // config = gateway config, json elements(?)
    // capability = filename/path, json format, jansson(?)

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
    ReturnCode readCapsFromFile(const std::string &filePath);

    /**
     * @brief Reads a directory and finds all json Service Manifest files
     * and adds the capabilities to the internal storage.
     * If a capability with the same name already exists in the storage
     * the gateways will be appended to the previously stored gatway list.
     *
     * @return ReturnCode::SUCCESS if successful
     * @return ReturnCode::FAILURE if parsing of file is unsuccessful
     *
     */
    ReturnCode readCapsFromDir(const std::string &dirPath);

    /**
     * @brief Returns all Gateways for a certain Capability.
     */
    std::map<std::string, json_t *> getGatewaysByCap(const std::string capID);

  private:

    std::vector<std::string> fileList(const std::string &filePath);
    bool is_json(const std::string &filename);

    std::map<std::string, std::map<std::string, json_t *>> m_configStore;
  };
}
