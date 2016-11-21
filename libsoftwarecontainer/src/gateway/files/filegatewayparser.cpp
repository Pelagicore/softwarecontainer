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

#include "filegatewayparser.h"
#include "jsonparser.h"

ReturnCode FileGatewayParser::parseFileGatewayConfigElement(const json_t *element,
                                                            FileSetting &setting)
{
    if (isError(parseObligatory(element, "path-host", setting.pathInHost))) {
        log_error() << "path-host key missing or of wrong type";
        return ReturnCode::FAILURE;
    }

    if (isError(parseObligatory(element, "path-container", setting.pathInContainer))) {
        log_error() << "path-container key missing or of wrong type";
        return ReturnCode::FAILURE;
    }

    if (setting.pathInHost.size() == 0) {
        log_error() << "path-host setting is an empty string";
        return ReturnCode::FAILURE;
    }

    if (setting.pathInContainer.size() == 0) {
        log_error() << "path-container setting is an empty string";
        return ReturnCode::FAILURE;
    }

    if (isError(parseOptional(element, "create-symlink", setting.createSymlinkInContainer))) {
        log_error() << "create-symlink has wrong format";
        return ReturnCode::FAILURE;
    }

    if (isError(parseOptional(element, "read-only", setting.readOnly))) {
        log_error() << "read-only has wrong format";
        return ReturnCode::FAILURE;
    }

    bool hasEnvVar = JSONParser::hasKey(element, "env-var-name");
    if (hasEnvVar) {
        if (isError(assumeFormat(element, "env-var-name", setting.envVarName))) {
            return ReturnCode::FAILURE;
        }
    }

    if (JSONParser::hasKey(element, "env-var-prefix")) {
        if (isError(assumeFormat(element, "env-var-prefix", setting.envVarPrefix))) {
            return ReturnCode::FAILURE;
        }

        if (!hasEnvVar) {
            log_error() << "Can't set env-var-prefix without env-var-name";
            return ReturnCode::FAILURE;
        }
    }

    if (JSONParser::hasKey(element, "env-var-suffix")) {
        if (isError(assumeFormat(element, "env-var-suffix", setting.envVarSuffix))) {
            return ReturnCode::FAILURE;
        }

        if (!hasEnvVar) {
            log_error() << "Can't set env-var-suffix without env-var-name";
            return ReturnCode::FAILURE;
        }
    }

    return ReturnCode::SUCCESS;
}

template<typename T>
ReturnCode FileGatewayParser::assumeFormat(const json_t *element, std::string key, T &result) {
    if (!JSONParser::read(element, key.c_str(), result)) {
        log_error() << "Key " << key << " has wrong format";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

template<typename T>
ReturnCode FileGatewayParser::parseObligatory(const json_t *element, std::string key, T &result)
{
    bool keyFound = JSONParser::read(element, key.c_str(), result);
    if (!keyFound) {
        log_error() << "Key " << key << " is obligatory";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

template<typename T>
ReturnCode FileGatewayParser::parseOptional(const json_t *element, std::string key, T &result)
{
    bool keyFound = JSONParser::hasKey(element, key.c_str());
    if (!keyFound) {
        return ReturnCode::SUCCESS;
    }

    return bool2ReturnCode(JSONParser::read(element, key.c_str(), result));
}
