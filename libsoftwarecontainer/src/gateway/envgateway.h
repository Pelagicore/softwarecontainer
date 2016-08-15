/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once
#include "gateway.h"

/*! Environment Gateway is used to define environment variables to the container
 *  ============================================================================
 *  The environment gateway allows users to specify environment variables that
 *  should be known to the container and all commands and functions running
 *  inside the container.
 *
 *  Required parameters:
 *  * name   string - The name of the environment variable in question
 *  * value  string - The value to attach to the name
 *
 *  Optional parameters:
 *  * append bool   - If the environment variable is already by the environment
 *                    gateway defined append the new value to the value already
 *                    defined. Defaults to false.
 *
 *  Example Configurations
 *  ======================
 *  \code{json}
 *  [
 *      {
 *          "name": "SOME_ENVIRONMENT_VARIABLE",
 *          "value": "SOME_VALUE"
 *      }
 *  ]
 *  \endcode
 *  Note that "value" will be read as a string
 *
 *  There are also the possibility to append to an already defined variable:
 *  \code{json}
 *  [
 *      {
 *          "name": "SOME_ENVIRONMENT_VARIABLE",
 *          "value": "SOME_SUFFIX",
 *          "append": true
 *      }
 *  ]
 *  \endcode
 *
 */
class EnvironmentGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("Env", "Environment gateway");

public:
    static constexpr const char *ID = "env";

    EnvironmentGateway();
    ~EnvironmentGateway();
    ReturnCode readConfigElement(const JSonElement &element);
    bool activateGateway();
    bool teardownGateway();

private:
    bool m_enabled = false;
    EnvironmentVariables m_variables;
};
