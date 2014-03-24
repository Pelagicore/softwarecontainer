/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "debug.h"
#include "generators.h"

#include "devicenodegateway.h"

DeviceNodeGateway::DeviceNodeGateway(ControllerAbstractInterface *controllerInterface):
    Gateway(controllerInterface)
{
}

std::string DeviceNodeGateway::id()
{
    return "devicenode";
}

std::string DeviceNodeGateway::environment()
{
    return "";
}

bool DeviceNodeGateway::setConfig(const std::string &config)
{
    log_debug("DeviceNodeGateway::setConfig called\n");

    bool success = true;

    return success;
}

bool DeviceNodeGateway::activate()
{
    bool success = false;

    return success;
}
