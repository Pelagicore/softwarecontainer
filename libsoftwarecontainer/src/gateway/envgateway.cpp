/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
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
