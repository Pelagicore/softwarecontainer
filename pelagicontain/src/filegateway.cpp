/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include <string>
#include "filegateway.h"

ReturnCode FileGateway::readConfigElement(const JSonElement &element)
{
    std::string pathInHost;
    std::string pathInContainer;
    bool createSymlinkInContainer = false;
    bool readOnly = false;
    std::string envVarName;
    std::string envVarValue;

    element.read("path-host", pathInHost);
    element.read("path-container", pathInContainer);
    element.read("create-symlink", createSymlinkInContainer);
    element.read("read-only", readOnly);
    element.read("env-var-name", envVarName);
    element.read("env-var-value", envVarValue);

    assert(pathInHost.size() != 0);

    // TODO : move mount to activate()

    std::string path;

    if (isDirectory(pathInHost))
        path = getContainer().bindMountFolderInContainer(pathInHost, pathInContainer, readOnly);
    else
        path = getContainer().bindMountFileInContainer(pathInHost, pathInContainer, readOnly);

    if (envVarName.size() != 0) {
        char value[1024];
        snprintf( value, sizeof(value), envVarValue.c_str(), path.c_str() );
        setEnvironmentVariable(envVarName, value);
    }

    if (createSymlinkInContainer) {
        createSymLinkInContainer(path, pathInHost);
    }

    return ReturnCode::SUCCESS;
}
