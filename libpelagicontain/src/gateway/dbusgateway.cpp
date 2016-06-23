/*
 *   Copyright (C) 2014-2016 Pelagicore AB
 *   All rights reserved.
 */
#include "dbusgateway.h"

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
    teardown();
}

static constexpr const char *SESSION_CONFIG = "dbus-gateway-config-session";
static constexpr const char *SYSTEM_CONFIG = "dbus-gateway-config-system";

ReturnCode DBusGateway::readConfigElement(const JSonElement &element)
{
    JSonElement sessionConfig = element[SESSION_CONFIG];
    if (sessionConfig.isValid() && sessionConfig.isArray()) {
        for(unsigned int i = 0; i < sessionConfig.elementCount(); i++) {
            JSonElement child = sessionConfig.arrayElementAt(i);
            json_array_append(m_sessionBusConfig, (json_t*)child.root());
        }
    }

    JSonElement systemConfig = element[SYSTEM_CONFIG];
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

    // set DBUS_{SESSION,SYSTEM}_BUS_ADDRESS env variable
    std::string variable = std::string("DBUS_")
                         + (m_type == SessionProxy ? "SESSION" : "SYSTEM")
                         + std::string("_BUS_ADDRESS");
    std::string value = "unix:path=/gateways/" + socketName();
    setEnvironmentVariable(variable, value);

    std::vector<std::string> commandVec = { "dbus-proxy"
                                          , m_socket
                                          , m_type == SessionProxy ? "session" : "system" };
    // Give the dbus-proxy access to the real dbus bus address.
    std::vector<std::string> envVec;
    if (char *envValue = getenv(variable.c_str())) {
        envVec.push_back(variable + "=" + envValue);
    } else if (m_type == SessionProxy) {
        log_error() << "Using DBus gateway in session mode"
                    << " and no " + variable + " set in host environment, dbus-proxy won't work";
        return false;
    } else {
        log_warn() << "Using DBus gateway in system mode"
                   << " and no " + variable + " set in host environment, this could be a problem";
    }

    // Spawn dbus-proxy with access to its stdin.
    try {
        Glib::spawn_async_with_pipes( "."
                                    , commandVec
                                    , envVec
                                    , Glib::SPAWN_STDOUT_TO_DEV_NULL // Redirect stdout
                                        | Glib::SPAWN_STDERR_TO_DEV_NULL // Redirect stderr
                                        | Glib::SPAWN_SEARCH_PATH // Search $PATH
                                    , sigc::slot<void>() // child setup
                                    , &m_pid
                                    , &m_infp // stdin
                                    );
    } catch (const Glib::Error &ex) {
        log_error() << "Failed to launch dbus-proxy";
        return false;
    }

    log_debug() << "Started dbus-proxy: " << m_pid;
    m_dbusProxyStarted = true;

    // Write configuration
    json_t *jsonConfig = json_object();
    json_object_set(jsonConfig, SESSION_CONFIG, m_sessionBusConfig);
    json_object_set(jsonConfig, SYSTEM_CONFIG, m_systemBusConfig);
    std::string config = std::string(json_dumps(jsonConfig, JSON_COMPACT));

    log_debug() << "Sending config " << config;
    unsigned int count = sizeof(char) * config.length();
    ssize_t written = write(m_infp, config.c_str(), count);

    // writing didn't work at all
    if (written == -1) {
        log_error() << "Failed to write to STDIN of dbus-proxy: " << strerror(errno);
        return false;
    } else if (written == (ssize_t)count) {
        // writing has written exact amout of bytes
        close(m_infp);
        m_infp = INVALID_FD;
        // dbus-proxy might take some time to create the bus socket
        if (isSocketCreated()) {
            log_debug() << "Found D-Bus socket: " << m_socket;
            return true;
        } else {
            log_error() << "Did not find any D-Bus socket: " << m_socket;
            return false;
        }
    } else {
        // something went wrong during the write
        log_error() << "Failed to write to STDIN of dbus-proxy!";
        return false;
    }
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
        if (m_pid != INVALID_PID && kill(m_pid, 0)) {
            log_debug() << "Killing dbus proxy with pid " << m_pid;
            kill(m_pid, SIGTERM);
            waitpid(m_pid, nullptr, WEXITED); // Wait until it exits
        }

        if (access(m_socket.c_str(), F_OK) != -1) {
            if (unlink(m_socket.c_str()) == -1) {
                log_error() << "Could not remove " << m_socket << ": " << strerror(errno);
                success = false;
            }
        }
    } else {
        log_warn() << "Trying to tear down dbus-gateway that has not been started";
    }

    return success;
}

std::string DBusGateway::socketName()
{
    // Return the filename after stripping directory info
    std::string socket(m_socket.c_str());
    return socket.substr(socket.rfind('/') + 1);
}



