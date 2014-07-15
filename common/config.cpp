/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include "config.h"

Config *Config::m_instance = 0;

Config::Config():
    m_ipcBufferSize(1024),
    m_controllerConnectionTimeout(10)
{
}

Config *Config::instance()
{
    if (!m_instance) {
        m_instance = new Config();
    }

    return m_instance;
}

int Config::ipcBufferSize()
{
    return m_ipcBufferSize;
}

int Config::controllerConnectionTimeout()
{
    return m_controllerConnectionTimeout;
}
