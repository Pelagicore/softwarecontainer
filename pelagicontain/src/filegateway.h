/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include <string>
#include <unistd.h>
#include "gateway.h"
#include "systemcallinterface.h"

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
        "env-var-value": "prefix-%s-suffix", // printf-like format string which specifies the value of the environment variable. The "%s" part of the string will be replaced at runtime by the actual location where the file is available in the container.
 */
class FileGateway :
    public Gateway
{

    LOG_DECLARE_CLASS_CONTEXT("File", "file gateway");

public:
    static constexpr const char *ID = "file";

    FileGateway()
    {
    }

    ~FileGateway()
    {
    }

    std::string id() override
    {
        return ID;
    }

    ReturnCode readConfigElement(JSonElement &element) override;

    bool activate() override
    {
        return true;
    }

    bool teardown() override
    {
        return true;
    }

private:
    bool m_enabled = false;
};
