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
    const std::string &containerRoot,
    const std::string &name):
    Gateway(controllerInterface),
    m_systemcallInterface(systemcallInterface),
    m_type(type),
    m_fp(NULL)
{
    if (m_type == SessionProxy) {
        m_socket = containerRoot
		    + std::string("/gateways/sess_")
            + name
            + std::string(".sock");
    } else {
        m_socket = containerRoot
            + std::string("/gateways/sys_")
            + name
            + std::string(".sock");
    }

    std::string variable;
    variable += "DBUS_";
    if (m_type == SessionProxy) {
        variable += "SESSION";
    } else {
        variable += "SYSTEM";
    }
    variable += "_BUS_ADDRESS";

    std::string value;
    value += "unix:path=";
    value += "/gateways/";
    value += socketName();

    controllerInterface->setEnvironmentVariable(variable, value);
}

DBusGateway::~DBusGateway()
{
    if(m_fp != NULL) {
        int exitCode;
        if(!m_systemcallInterface->makePcloseCall(&m_fp, exitCode)) {
            log_error("pclose() returned error: %i\n", exitCode);
        }
    }
}

std::string DBusGateway::id()
{
    return "dbus-proxy";
}

bool DBusGateway::setConfig(const std::string &config)
{
    m_config = config;

    if(m_config.length() > 1) {
      return true;
    }

    return false;
}

bool DBusGateway::activate()
{
    std::string command = "dbus-proxy ";
    command += m_socket + " " + typeString();

	std::string type("w");
    m_systemcallInterface->makePopenCall(command, type, &m_fp);

    if(m_fp == NULL) {
        return false;
    }

    const char *config = m_config.c_str();
    fwrite(config, sizeof(char), sizeof(config), m_fp);

    return true;
}

const char *DBusGateway::typeString()
{
    if (m_type == SessionProxy) {
        return "session";
    } else {
        return "system";
    }
}

const char *DBusGateway::socketName()
{
    // Return the filename after stripping directory info
    std::string socket(m_socket.c_str());
    return socket.substr(socket.rfind('/') + 1).c_str();
}

std::string DBusGateway::environment()
{
  /*
    log_debug("Requesting environment for %s with socket %s",
        typeString(), m_socket.c_str());

    std::string env;
    env += "DBUS_";
    if (m_type == SessionProxy) {
        env += "SESSION";
    } else {
        env += "SYSTEM";
    }
    env += "_BUS_ADDRESS=unix:path=";
    env += "/deployed_app/";
    env += socketName();
  */
    return "";
}
