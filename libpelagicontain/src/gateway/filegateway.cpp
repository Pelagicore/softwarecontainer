/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include <string>
#include "filegateway.h"

FileGateway::FileGateway()
    : Gateway(ID)
    , m_settings({})
{
}

ReturnCode FileGateway::readConfigElement(const JSonElement &element)
{
    FileSetting setting;
    element.read("path-host", setting.pathInHost);
    element.read("path-container", setting.pathInContainer);
    element.read("create-symlink", setting.createSymlinkInContainer);
    element.read("read-only", setting.readOnly);
    element.read("env-var-name", setting.envVarName);
    assert(setting.pathInHost.size() != 0);
    element.read("env-var-prefix", setting.envVarPrefix);
    element.read("env-var-suffix", setting.envVarSuffix);

    m_settings.push_back(setting);
    return ReturnCode::SUCCESS;
}

bool FileGateway::activate()
{
    for (FileSetting &setting : m_settings) {
        std::string path;

        if (isDirectory(setting.pathInHost)) {
            path = getContainer().bindMountFolderInContainer(setting.pathInHost
                    , setting.pathInContainer
                    , setting.readOnly);
        } else {
            path = getContainer().bindMountFileInContainer(setting.pathInHost
                    , setting.pathInContainer
                    , setting.readOnly);
        }

        if (setting.envVarName.size() != 0) {
            char value[1024];
            snprintf(value, sizeof(value), setting.envVarValue.c_str(), path.c_str());
            setEnvironmentVariable(setting.envVarName, value);
        }

            if (path.size() == 0) {
               log_error() << "Bind mount failed";
               return false;
            }

            if (setting.envVarName.size() != 0) {
                std::string value = StringBuilder() << setting.envVarPrefix << path << setting.envVarSuffix;
                setEnvironmentVariable(setting.envVarName, value);
            }

            if (setting.createSymlinkInContainer) {
                getContainer().createSymLink(getContainer().rootFS() + setting.pathInHost, path);
            }
        }
    }

    return true;
}
