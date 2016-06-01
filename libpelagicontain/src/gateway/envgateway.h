/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once
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

    EnvironmentGateway();
    ~EnvironmentGateway();
    ReturnCode readConfigElement(const JSonElement &element) override;
    bool activate() override;

private:
    bool m_enabled = false;
    EnvironmentVariables m_variables;
};
