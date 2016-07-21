/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include <string>
#include <unistd.h>
#include "gateway.h"

/**
 * That gateway lets you map files (including socket files) or folders from the host into the container's filesystem.
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

    ReturnCode readConfigElement(const JSonElement &element) override;
    bool activate() override;

private:
    bool m_hasBeenConfigured = false;

    struct FileSetting {
        std::string pathInHost;
        std::string pathInContainer;
        bool createSymlinkInContainer;
        bool readOnly;
        std::string envVarName;
        std::string envVarPrefix;
        std::string envVarSuffix;
    };

    std::vector<FileSetting> m_settings;
};
