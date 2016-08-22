/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include <string>
#include <unistd.h>
#include "waylandgateway.h"

// TODO: no clue why the following lines are needed
constexpr const char *WaylandGateway::SOCKET_FILE_NAME;
constexpr const char *WaylandGateway::WAYLAND_RUNTIME_DIR_VARIABLE_NAME;

WaylandGateway::WaylandGateway() :
    Gateway(ID)
{
}

WaylandGateway::~WaylandGateway()
{
}


ReturnCode WaylandGateway::readConfigElement(const JSonElement &element)
{
    bool enabled;
    element.readBoolean(ENABLED_FIELD, enabled);
    m_enabled = enabled;
    return ReturnCode::SUCCESS;
}

bool WaylandGateway::activateGateway()
{
    if (m_enabled) {
        const char *dir = getenv(WAYLAND_RUNTIME_DIR_VARIABLE_NAME);
        if (dir != nullptr) {
            log_info() << "enabling Wayland gateway. Socket dir:" << dir;
            std::string d = logging::StringBuilder() << dir << "/" << SOCKET_FILE_NAME;
            std::string path;
            ReturnCode result = getContainer()->bindMountFileInContainer(d, SOCKET_FILE_NAME, path, false);
            if (isError(result)) {
                log_error() << "Could not bind mount the wayland socket into the container";
                return false;
            }

            setEnvironmentVariable(WAYLAND_RUNTIME_DIR_VARIABLE_NAME, parentPath(path));
        } else {
            log_error() << "Should enable wayland gateway, but " << WAYLAND_RUNTIME_DIR_VARIABLE_NAME << " is not defined";
            return false;
        }
    } else {
        log_info() << "Wayland gateway disabled";
    }
    return true;
}

bool WaylandGateway::teardownGateway()
{
    return true;
}
