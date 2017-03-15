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


#pragma once

#include <jansson.h>

#include "softwarecontainer-common.h"
#include "servicemanifestloader.h"
#include "configstoreerror.h"

namespace softwarecontainer {

class ServiceManifestFileLoader : public ServiceManifestLoader
{

LOG_DECLARE_CLASS_CONTEXT("SMFL", "SoftwareContainer service manifest loader");

public:
    // Constructor just needs to init parent with the service manifest source string
    ServiceManifestFileLoader(const std::string &source);

    virtual std::vector<json_t *> loadContent() override;

private:

    /**
     * @brief Reads a directory and finds all json Service Manifest files.
     *
     * @throws ServiceManifestPathError if the path to the Service Manifest(s) is not allowed
     *
     */
    void loadServiceManifestDir();

    /**
     * @brief Reads a Service Manifest file, of type json, and adds the
     * capabilities to the internal storage.
     * If a capability with the same name already exists in the storage
     * the gateways will be appended to the previously stored gatway list.
     *
     * @throws ServiceManifestParseError if parsing of the file is unsuccessful
     * @throws CapabilityParseError if parsing of one or more Capabilities in file
     * is unsuccessful
     *
     */
    void loadServiceManifestFile(const std::string &filePath);

    /**
     * @brief Iterates through the files in the source and finds all json files.
     *
     * @return a vector of all json files present in the directory, an empty list if no
     * json files can be found in the directory
     */
    std::vector<std::string> fileList();

    /**
     * @brief Checks if a file name ends in ".json"
     *
     * @return true/false
     */
    bool isJsonFile(const std::string &filename);

};

} // namespace softwarecontainer
