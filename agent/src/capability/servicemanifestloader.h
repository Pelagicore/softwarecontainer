/*
 * Copyright (C) 2017 Pelagicore AB
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

#include "softwarecontainer-common.h"

#include <jsonparser.h>

namespace softwarecontainer {

/**
 * Base class used for creating a service manifest loader.
 *
 * ConfigStore uses a loader derived from this class to load service manifests.
 */
class ServiceManifestLoader
{
public:
    // Enforce inheriting classes to initialize service manifest source member
    ServiceManifestLoader() = delete;

    /**
     * @brief Constructs a service manifest loader object and stores the source string
     * @param source The source of the service manifest
     */
    ServiceManifestLoader(const std::string &source) : m_source(source) {}

    /**
     * @brief The destructor decref:s the json items in the stored content
     */
    virtual ~ServiceManifestLoader() {
        for (json_t *item : m_content) {
            json_decref(item);
        }
    }

    /**
     * @brief Loads the json content of the service manifest(s) based on the source
     * sent to the constructor.
     *
     * @return a vector of json_t corresponding to the content of the service manifest
     * @throws ConfigStoreError if loading of the content was unsuccessful
     */
    virtual std::vector<json_t *> loadContent() = 0;

protected:
    // A vector of json_t representation(s) of the service manifest(s)
    std::vector<json_t *> m_content;

    // The string source of the service manifest(s)
    const std::string m_source;
};

} // namespace softwarecontainer
