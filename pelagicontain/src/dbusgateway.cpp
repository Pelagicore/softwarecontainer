/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "dbusgateway.h"
#include "pelagicontain-common.h"

DBusGateway::DBusGateway(ProxyType type, const std::string &gatewayDir, const std::string &name) :
    Gateway(), m_type(type), m_pid(-1), m_infp(-1), m_outfp(-1),
    m_hasBeenConfigured(false), m_dbusProxyStarted(false)
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
    return ID;
}

bool DBusGateway::setConfig(const std::string &config)
{
    JSonElement rootElement(config);

    if ( !rootElement.isArray() ) {
        return false;
    }

    std::vector<JSonElement> elements;
    rootElement.read(elements);

    if (elements.size() > 1) {
        // TODO : remove that limitation
        log_error() << "The DBUS gateway currently supports only 1 configuration snippet";
        return false;
    }

    if (elements.size() == 1) {
        m_config = elements[0].dump();
    }

    if (m_config.length() > 1) {
        m_hasBeenConfigured = true;
        return true;
    }

    return false;
}

bool DBusGateway::activate()
{
    if (!m_hasBeenConfigured) {
        log_warning() << "'Activate' called on non-configured gateway " << id();
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
    setEnvironmentVariable(variable, value);

    // Open pipe
    std::string command = "dbus-proxy ";
    command += m_socket + " " + typeString();
    log_debug() << command;
    m_pid = makePopenCall(command, &m_infp, &m_outfp);
    if (m_pid == -1) {
        log_error() << "Failed to launch " << command;
        return false;
    } else {
        log_debug() << "Started dbus-proxy: " << m_pid;
        m_dbusProxyStarted = true;
    }

    size_t count = sizeof(char) * m_config.length();

    ssize_t written = write(m_infp,
            m_config.c_str(),
            count);

    // writing didn't work at all
    if (written == -1) {
        log_error() << "Failed to write to STDIN of dbus-proxy: " << strerror(errno);
        return false;
    }

    // writing has written exact amout of bytes
    if (written == (ssize_t)count) {
        close(m_infp);
        m_infp = -1;
        // dbus-proxy might take some time to create the bus socket
        if ( isSocketCreated() ) {
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
        if (m_pid != -1) {
            if ( !makePcloseCall(m_pid, m_infp, m_outfp) ) {
                log_error() << "makePcloseCall() returned error";
                success = false;
            }
        } else {
            log_error() << "Failed to close connection to dbus-proxy!";
            success = false;
        }

        if (unlink( m_socket.c_str() ) == -1) {
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
    std::string socket( m_socket.c_str() );
    return socket.substr(socket.rfind('/') + 1);
}
