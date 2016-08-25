
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

ReturnCode EnvironmentGateway::readConfigElement(const JSonElement &element)
{
    std::string variableName;
    element.read("name", variableName);

    if (variableName.empty()) {
        log_warning() << "No 'name' specified for EnvironmentGateway element, aborting";
        return ReturnCode::FAILURE;
    }

    std::string variableValue;
    element.read("value", variableValue);

    if (variableValue.empty()) {
        log_warning() << "No 'value' specified for EnvironmentGateway element, aborting";
        return ReturnCode::FAILURE;
    }

    bool appendMode = false;
    element.read("append", appendMode);

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
