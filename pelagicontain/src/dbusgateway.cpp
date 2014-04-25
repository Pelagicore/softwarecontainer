/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include "dbusgateway.h"
#include "debug.h"

DBusGateway::DBusGateway(ControllerAbstractInterface *controllerInterface,
                         SystemcallAbstractInterface *systemcallInterface,
                         ProxyType type,
                         const std::string &gatewayDir,
                         const std::string &name):
    Gateway(controllerInterface),
    m_systemcallInterface(systemcallInterface),
    m_type(type),
    m_pid(-1),
    m_infp(-1),
    m_outfp(-1),
    m_hasBeenConfigured(false)
{
    if (m_type == SessionProxy) {
        m_socket = gatewayDir
            + std::string("/sess_")
            + name
            + std::string(".sock");
    } else {
        m_socket = gatewayDir
            + std::string("/sys_")
            + name
            + std::string(".sock");
    }
}

DBusGateway::~DBusGateway()
{
}

std::string DBusGateway::id()
{
    return "dbus-proxy";
}

bool DBusGateway::setConfig(const std::string &config)
{
    m_config = config;

    if(m_config.length() > 1) {
        m_hasBeenConfigured = true;
        return true;
    }

    return false;
}

bool DBusGateway::activate()
{
    if(!m_hasBeenConfigured) {
        log_error ("'Activate' called on non-configured gateway %s",
                   id().c_str());
        return false;
    }

    // set DBUS ENV
    std::string variable = "DBUS_";
    if (m_type == SessionProxy) {
        variable += "SESSION";
    } else {
        variable += "SYSTEM";
    }
    variable += "_BUS_ADDRESS";

    std::string value = "unix:path=/gateways/";
    value += socketName();
    m_controllerInterface->setEnvironmentVariable(variable, value);

    // Open pipe
    std::string command = "dbus-proxy ";
    command += m_socket + " " + typeString();
    m_pid = m_systemcallInterface->makePopenCall(command,
                                                 &m_infp,
                                                 &m_outfp);
    if(m_pid == -1) {
        log_error ("Failed to launch %s", command.c_str());
        return false;
    }

    size_t count = sizeof(char) * m_config.length();

    ssize_t written = write(m_infp,
                            m_config.c_str(),
                            count);

    // writing didn't work at all
    if(written == -1) {
        log_error ("Failed to write to STDIN of dbus-proxy!");
        return false;
    }

    // writing has written exact amout of bytes
    if(written == (ssize_t)count) {
        return true;
    }

    // something went wrong during the write
    log_error ("Failed to write to STDIN of dbus-proxy!");

    return false;
}

bool DBusGateway::teardown() {

    bool success = true;

    if(m_pid != -1) {
        if(!m_systemcallInterface->makePcloseCall(m_pid, m_infp, m_outfp)) {
            log_error("makePcloseCall() returned error\n");
            success = false;
        }
    } else {
        log_error("Failed to close connection to dbus-proxy!");
        success = false;
    }

    if(unlink(m_socket.c_str()) == -1) {
        log_error("Could not remove %s\n", m_socket.c_str());
        success = false;
    }

    return success;
}

const char *DBusGateway::typeString()
{
    if (m_type == SessionProxy) {
        return "session";
    } else {
        return "system";
    }
}

std::string DBusGateway::socketName()
{
    // Return the filename after stripping directory info
    std::string socket(m_socket.c_str());
    return socket.substr(socket.rfind('/') + 1);
}

std::string DBusGateway::environment()
{
    return "";
}
