
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
    std::string variableName;
    std::string variableValue;

    if (!read(element, "name", variableName)) {
        log_error() << "Key \"name\" missing or not a string in json configuration";
        return ReturnCode::FAILURE;
    }

    if (variableName.length() == 0) {
        log_error() << "Value for \"name\" key is an empty string";
        return ReturnCode::FAILURE;
    }

    if (!read(element, "value", variableValue)) {
        log_error() << "Key \"value\" missing or not a string in json configuration";
        return ReturnCode::FAILURE;
    }

    if (variableValue.length() == 0) {
        log_error() << "Value for \"value\" key is an empty string";
        return ReturnCode::FAILURE;
    }

    bool appendMode = false;
    read(element, "append", appendMode); // This key is optional

    if (m_variables.count(variableName) == 0) {
        m_variables[variableName] = variableValue;
    } else {
        if (appendMode) {
            m_variables[variableName] += variableValue;
        } else {
            log_error() << "Env variable " << variableName << " already defined with value : " << m_variables[variableName];
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
