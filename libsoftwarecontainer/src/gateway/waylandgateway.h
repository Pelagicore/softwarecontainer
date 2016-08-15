/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include "gateway.h"

/**
 * To be removed
 */
class WaylandGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("Wayl", "Wayland gateway");

public:
    static constexpr const char *ID = "wayland";
    static constexpr const char *WAYLAND_RUNTIME_DIR_VARIABLE_NAME = XDG_RUNTIME_DIR_VARIABLE_NAME;
    static constexpr const char *SOCKET_FILE_NAME = "wayland-0";

    WaylandGateway();
    ~WaylandGateway();

    ReturnCode readConfigElement(const JSonElement &element);
    virtual bool activateGateway();
    virtual bool teardownGateway();

private:
    bool m_enabled = false;
};
