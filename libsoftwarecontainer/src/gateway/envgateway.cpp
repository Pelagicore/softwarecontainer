
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
#include <unistd.h>
#include "envgateway.h"

EnvironmentGateway::EnvironmentGateway() :
    Gateway(ID)
{
}

EnvironmentGateway::~EnvironmentGateway()
{
}

ReturnCode EnvironmentGateway::readConfigElement(const json_t *element)
{
    json_t *variableName = json_object_get(element, "name");
    if (!variableName) {
        log_error() << "Key \"name\" missing in json configuration";
        return ReturnCode::FAILURE;
    }

    if (!json_is_string(variableName)) {
        log_error() << "Value for \"name\" key is not a string";
        return ReturnCode::FAILURE;
    }

    if (json_string_length(variableName) == 0) {
        log_error() << "Value for \"name\" key is an empty string";
        return ReturnCode::FAILURE;
    }

    json_t *variableValue = json_object_get(element, "value");
    if (!variableValue) {
        log_error() << "Key \"value\" missing in json configuration";
        return ReturnCode::FAILURE;
    }

    if (!json_is_string(variableValue)) {
        log_error() << "Value for \"value\" key is not a string";
        return ReturnCode::FAILURE;
    }

    if (json_string_length(variableValue) == 0) {
        log_error() << "Value for \"value\" key is an empty string";
        return ReturnCode::FAILURE;
    }

    bool appendMode = false;
    json_t *appendModeValue = json_object_get(element, "append");
    if (appendModeValue) {
        if (json_is_boolean(appendModeValue)) {
            appendMode = json_is_true(appendModeValue);
        } else {
            log_error() << "Key \"append\" not a bool";
            return ReturnCode::FAILURE;
        }
    }

    std::string realVariableName = json_string_value(variableName);
    std::string realVariableValue = json_string_value(variableValue);

    if (m_variables.count(realVariableName) == 0) {
        m_variables[realVariableName] = realVariableValue;
    } else {
        if (appendMode) {
            m_variables[realVariableName] += realVariableValue;
        } else {
            log_error() << "Env variable " << realVariableName << " already defined with value : " << m_variables[realVariableName];
            return ReturnCode::FAILURE;
        }
    }

    return ReturnCode::SUCCESS;
}

bool EnvironmentGateway::activateGateway()
{
    if (m_variables.empty()) {
        log_error() << "No environment variables set in gateway";
        return false;
    }

    for (auto &variable : m_variables) {
        if (isError(setEnvironmentVariable(variable.first, variable.second))) {
            log_error() << "Could not set environment variable " << variable.first << " for the container";
            return false;
        }
    }

    return true;
}

bool EnvironmentGateway::teardownGateway()
{
    return true;
}
