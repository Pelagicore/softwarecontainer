/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "dbusgateway.h"
#include "pelagicontain-common.h"

DBusGateway::DBusGateway(ProxyType type
                       , const std::string &gatewayDir
                       , const std::string &name)
    : Gateway(ID)
    , m_type(type)
    , m_hasBeenConfigured(false)
    , m_dbusProxyStarted(false)
{
    m_socket = gatewayDir 
             + (m_type == SessionProxy ? "/sess_" : "/sys_")
             + name 
             + ".sock";

    m_sessionBusConfig = json_array();
    m_systemBusConfig = json_array();
}

DBusGateway::~DBusGateway()
{
    if (m_pid != INVALID_PID) {
        log_debug() << "Killing dbus proxy with pid " << m_pid;
        kill(m_pid, SIGTERM);
    }

    // TODO : fix dbus-proxy to delete the socket files when terminating
}

static constexpr const char *SESSION_CONFIG = "dbus-gateway-config-session";
static constexpr const char *SYSTEM_CONFIG = "dbus-gateway-config-system";

ReturnCode DBusGateway::readConfigElement(const JSonElement &element)
{
    auto sessionConfig = element[SESSION_CONFIG];
    if (sessionConfig.isValid() && sessionConfig.isArray()) {
        for(unsigned int i = 0; i < sessionConfig.elementCount(); i++) {
            JSonElement child = sessionConfig.arrayElementAt(i);
            json_array_append(m_sessionBusConfig, (json_t*)child.root());
        }
    }

    auto systemConfig = element[SYSTEM_CONFIG];
    if (systemConfig.isValid() && systemConfig.isArray()) {
        for(unsigned int i = 0; i < systemConfig.elementCount(); i++) {
            JSonElement child = systemConfig.arrayElementAt(i);
            json_array_append(m_systemBusConfig, (json_t*)child.root());
        }
    }

    m_hasBeenConfigured = true;
    return ReturnCode::SUCCESS;
}

bool DBusGateway::activate()
{
    if (!m_hasBeenConfigured) {
        log_warning() << "'Activate' called on non-configured gateway " << id();
        return false;
    }

    // set DBUS ENV var
    std::string variable = "DBUS_";
    variable += m_type == SessionProxy ? "SESSION" : "SYSTEM";
    variable += "_BUS_ADDRESS";
    std::string value = "unix:path=/gateways/" + socketName();
    setEnvironmentVariable(variable, value);

    // Open pipe
    std::string command = "dbus-proxy " + m_socket + " " + typeString();
    log_debug() << command;
    auto code = makePopenCall(command, m_infp, m_pid);
    if (isError(code)) {
        log_error() << "Failed to launch " << command;
        return false;
    } else {
        log_debug() << "Started dbus-proxy: " << m_pid;
        m_dbusProxyStarted = true;
    }

    // Write configuration
    json_t *jsonConfig = json_object();
    json_object_set(jsonConfig, SESSION_CONFIG, m_sessionBusConfig);
    json_object_set(jsonConfig, SYSTEM_CONFIG, m_systemBusConfig);
    std::string config = std::string(json_dumps(jsonConfig, JSON_COMPACT));

    log_debug() << "Sending config " << config;
    auto count = sizeof(char) * config.length();
    auto written = write(m_infp,
            config.c_str(),
            count);

    // writing didn't work at all
    if (written == -1) {
        log_error() << "Failed to write to STDIN of dbus-proxy: " << strerror(errno);
        return false;
    }

    // writing has written exact amout of bytes
    if (written == (ssize_t)count) {
        close(m_infp);
        m_infp = INVALID_FD;
        // dbus-proxy might take some time to create the bus socket
        if (isSocketCreated()) {
            log_debug() << "Found D-Bus socket: " << m_socket;
        } else {
            log_error() << "Did not find any D-Bus socket: " << m_socket;
            return false;
        }
        return true;
    }

    // something went wrong during the write
    log_error() << "Failed to write to STDIN of dbus-proxy!";

    return false;
}

bool DBusGateway::isSocketCreated() const
{
    int maxCount = 1000;
    int count = 0;
    do {
        if (count >= maxCount) {
            return false;
        }
        count++;
        usleep(1000 * 10);
    } while (access(m_socket.c_str(), F_OK) == -1);
    return true;
}

bool DBusGateway::teardown()
{
    bool success = true;

    if (m_dbusProxyStarted) {
        if (m_pid != INVALID_PID) {
            if (!makePcloseCall(m_pid, m_infp)) {
                log_error() << "makePcloseCall() returned error";
                success = false;
            }
        } else {
            log_error() << "Failed to close connection to dbus-proxy!";
            success = false;
        }

        if (unlink(m_socket.c_str()) == -1) {
            log_error() << "Could not remove " << m_socket << ": " << strerror(errno);
            success = false;
        }
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



