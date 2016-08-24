
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


#include <string>
#include "filegateway.h"

FileGateway::FileGateway()
    : Gateway(ID)
    , m_settings({})
{
}

ReturnCode FileGateway::readConfigElement(const json_t *element)
{
    FileSetting setting;

    typedef std::pair<const char *, std::string *> SettingStringPair;
    typedef std::pair<const char *, bool *> SettingBoolPair;

    const SettingStringPair stringMapping[] = {
        SettingStringPair("path-host", &setting.pathInHost),
        SettingStringPair("path-container", &setting.pathInContainer),
        SettingStringPair("env-var-name", &setting.envVarName),
        SettingStringPair("env-var-prefix", &setting.envVarPrefix),
        SettingStringPair("env-var-suffix", &setting.envVarSuffix)
    };

    const SettingBoolPair boolMapping[] = {
        SettingBoolPair("create-symlink", &setting.createSymlinkInContainer),
        SettingBoolPair("read-only", &setting.readOnly)
    };

    for (SettingStringPair s : stringMapping) {
        const char *key = s.first;
        json_t *settingString = json_object_get(element, key);
        if (settingString) {
            if (json_is_string(settingString)) {
                std::string *structPart = s.second;
                *structPart = json_string_value(settingString);
            } else {
                log_error() << "Value for " << key << " key is not a string";
                return ReturnCode::FAILURE;
            }
        }
    }

    for (SettingBoolPair b : boolMapping) {
        const char *key = b.first;
        json_t *settingBool = json_object_get(element, key);
        if (settingBool) {
            bool *structPart = b.second;
            if (json_is_boolean(settingBool)) {
                *structPart = json_is_true(settingBool);
            } else {
                log_error() << "Value for " << key << " key is not a bool";
                return ReturnCode::FAILURE;
            }
        }
    }

    if (setting.pathInHost.size() == 0) {
        log_error() << "FileGateway config is lacking 'path-host' setting";
        return ReturnCode::FAILURE;
    }

    if (setting.pathInContainer.size() == 0) {
        log_error() << "FileGateway config is lacking 'path-container' setting";
        return ReturnCode::FAILURE;
    }

    m_settings.push_back(setting);
    return ReturnCode::SUCCESS;
}

bool FileGateway::activateGateway()
{
    if (m_settings.size() > 0) {
        for (FileSetting &setting : m_settings) {
            std::string path;

            if (isDirectory(setting.pathInHost)) {
                ReturnCode result = getContainer()->bindMountFolderInContainer(setting.pathInHost
                                                                             , setting.pathInContainer
                                                                             , path
                                                                             , setting.readOnly);
                if (isError(result)) {
                    log_error() << "Could not bind mount folder into container";
                    return false;
                }
            } else {
                ReturnCode result = getContainer()->bindMountFileInContainer(setting.pathInHost
                                                                           , setting.pathInContainer
                                                                           , path
                                                                           , setting.readOnly);
                if (isError(result)) {
                    log_error() << "Could not bind mount file into container";
                    return false;
                }
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
                getContainer()->createSymLink(getContainer()->rootFS() + setting.pathInHost, path);
            }
        }
        return true;
    }

    return false;
}

bool FileGateway::teardownGateway()
{
    return true;
}
