
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
#include <ivi-profiling.h>

//#include "SoftwareContainerAgent_dbuscpp_adaptor.h"
#include "softwarecontainer-common.h"

#include <jsonparser.h>
#include "commandjob.h"


namespace softwarecontainer {

class ConfigStore
{
    LOG_DECLARE_CLASS_CONTEXT("CS", "ConfigStore");

public:
    ConfigStore();
    ~ConfigStore();

    // Fetch by capability ID
    // config = gateway config, json elements(?)
    // capability = filename/path, json format, jansson(?)

    /**
     * @brief Fetches capabilities by ID from a Service Manifest file.
     * 
     * Each returned Gateway object represents a 'gateway' entry in
     * the json file.
     */
    static std::vector<GatewayConfiguration> capFromFile(std::string capID, std::string &filePath);

    /**
     * @brief Fetches capabilities by ID from a directory with Service Manifest files.
     * 
     * Each returned Gateway object represents a 'gateway' entry in
     * the json file.
     *                                                                                                                          
     */
    static std::vector<GatewayConfiguration> capFromDir(std::string capID, std::string &configDirPath);

private:

    /**
     * @brief Returns all Gateways for a certain Capability.
     */
    static std::vector<GatewayConfiguration> getGatewaysByCap(std::string capID, const std::string &filePath);

    /**
     * @brief Populates a vector with Gateway objects and returns true on
     * 	success and false on failure.
     */
    static bool parseGatewayData(const pelagicore::JSonElement &gatewayData,
				 std::vector<GatewayConfiguration> &gateways,
				 std::string *error);

    static std::vector<std::string> configFileList(const std::string &configPath);
    static bool hasEnding(std::string const &fullString, std::string const &ending);
    
};
}
