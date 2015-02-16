/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include <string>
#include <unistd.h>
#include "gateway.h"

/**
 * This gateway can be used to define environment variables in the container
 */
class EnvironmentGateway :
    public Gateway
{

    LOG_DECLARE_CLASS_CONTEXT("Env", "Environment gateway");

public:
    static constexpr const char *ID = "env";

    EnvironmentGateway() :
        Gateway(ID)
    {
    }

    ReturnCode readConfigElement(const JSonElement &element) override
    {
        std::string variableName;
        element.read("name", variableName);
        std::string variableValue;
        element.read("value", variableValue);
        m_variables[variableName] = variableValue;
        return ReturnCode::SUCCESS;
    }

    bool activate() override
    {
        for (auto &variable : m_variables) {
            setEnvironmentVariable(variable.first, variable.second);
        }
        return true;
    }

private:
    bool m_enabled = false;
    EnvironmentVariables m_variables;
};
