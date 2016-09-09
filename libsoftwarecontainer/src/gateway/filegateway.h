
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


#pragma once

#include <string>
#include <unistd.h>
#include "gateway.h"

/**
 * This gateway lets you map files (including socket files) or folders from the host into the container's filesystem.
 * In the container, the files are mapped into a subfolders (currently "/gateways"), at the location specified by the "path-container" field (see below).
   Example:
{
        "path-host": "/tmp/someIPSocket",   // Path to the file in host's file-system
        "path-container": "someIPSocket",   // Sub-path of the mount point in the container
        "create-symlink": true, // specifies whether a symbolic link should to be created so that the file is available in the container under the same path is in the host.
        "read-only": false,  // if true, the file is accessible in read-only mode in the container
        "env-var-name": "SOMEIP_SOCKET_PATH", // name of a environment variable to be set
        "env-var-prefix": "some-path-prefix", // define a prefix for the path set in the environment variable defined by "env-var-name"
        "env-var-suffix": "some-path-suffix", // define a suffix for the path set in the environment variable defined by "env-var-name"
 */
class FileGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("File", "file gateway");

public:
    static constexpr const char *ID = "file";

    FileGateway();

    ReturnCode readConfigElement(const json_t *element) override;
    bool activateGateway() override;
    bool teardownGateway() override;

private:
    struct FileSetting {
        std::string pathInHost;
        std::string pathInContainer;
        bool createSymlinkInContainer;
        bool readOnly;
        std::string envVarName;
        std::string envVarPrefix;
        std::string envVarSuffix;
    };
    virtual std::string bindMount(const FileSetting &setting);

    std::vector<FileSetting> m_settings;
};
