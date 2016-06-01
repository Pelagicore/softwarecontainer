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

    std::string variableValue;
    element.read("value", variableValue);

    bool appendMode = false;
    element.read("append", appendMode);

    if (m_variables.count(variableName) == 0) {
        m_variables[variableName] = variableValue;
    } else {
        if (appendMode) {
            m_variables[variableName] += variableValue;
        } else {
            log_error() << "Env variable " << variableName << " already defined with value : " << m_variables[variableName];
        }
    }

    return ReturnCode::SUCCESS;
}

bool EnvironmentGateway::activate()
{
    for (auto &variable : m_variables) {
        setEnvironmentVariable(variable.first, variable.second);
    }
    return true;
}
