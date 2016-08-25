
/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */


#include "dbusgateway.h"

DBusGateway::DBusGateway(ProxyType type
                       , const std::string &gatewayDir
                       , const std::string &name)
    : Gateway(ID)
    , m_type(type)
{
    m_state = GatewayState::CREATED;
    m_socket = gatewayDir 
             + (m_type == SessionProxy ? "/sess_" : "/sys_")
             + name 
             + ".sock";

    m_sessionBusConfig = json_array();
    m_systemBusConfig = json_array();
}

DBusGateway::~DBusGateway()
{
    if (m_state == GatewayState::ACTIVATED) {
        teardown();
    }
}

static constexpr const char *SESSION_CONFIG = "dbus-gateway-config-session";
static constexpr const char *SYSTEM_CONFIG = "dbus-gateway-config-system";

ReturnCode DBusGateway::readConfigElement(const json_t *element)
{
    bool has_session, has_system = false;

    // First, parse session bus configuration
    json_t *sessionConfig = json_object_get(element, SESSION_CONFIG);
    if (sessionConfig) {
        has_session = true;
        if (!json_is_array(sessionConfig)) {
            log_error() << "Value for " << SESSION_CONFIG << " is not an array";
            return ReturnCode::FAILURE;
        }

        for (unsigned int i = 0; i < json_array_size(sessionConfig); i++) {
            json_t *child = json_array_get(sessionConfig, i);
            // TODO: Error checking here, so that dbus-proxy can accept config
            json_array_append(m_sessionBusConfig, json_deep_copy(child));
        }
    }

    // Then, parse system bus configuration
    json_t *systemConfig = json_object_get(element, SYSTEM_CONFIG);
    if (systemConfig) {
        has_system = true;
        if (!json_is_array(systemConfig)) {
            log_error() << "Value for " << SYSTEM_CONFIG << " is not an array";
            return ReturnCode::FAILURE;
        }

        for (unsigned int i = 0; i < json_array_size(systemConfig); i++) {
            json_t *child = json_array_get(systemConfig, i);
            // TODO: Error checking here, so that dbus-proxy can accept config
            json_array_append(m_systemBusConfig, json_deep_copy(child));
        }
    }

    if (!has_session && !has_system) {
        log_error() << "Neither system nor session configuration was provided";
        return ReturnCode::FAILURE;
    }

    m_state = GatewayState::CONFIGURED;
    return ReturnCode::SUCCESS;
}

bool DBusGateway::activateGateway()
{
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

    if (!startDBusProxy(commandVec, envVec)) {
        return false;
    }

    // Write configuration
    json_t *jsonConfig = json_object();
    json_object_set(jsonConfig, SESSION_CONFIG, m_sessionBusConfig);
    json_object_set(jsonConfig, SYSTEM_CONFIG, m_systemBusConfig);
    std::string config = std::string(json_dumps(jsonConfig, JSON_COMPACT));

    return testDBusConnection(config);
}

bool DBusGateway::testDBusConnection(const std::string &config)
{
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

bool DBusGateway::startDBusProxy(const std::vector<std::string> &commandVec, const std::vector<std::string> &envVec)
{
    // Spawn dbus-proxy with access to its stdin.
    try {
        Glib::spawn_async_with_pipes( "."
                                    , commandVec
                                    , envVec
                                    , Glib::SPAWN_STDOUT_TO_DEV_NULL // Redirect stdout
                                        | Glib::SPAWN_STDERR_TO_DEV_NULL // Redirect stderr
                                        | Glib::SPAWN_SEARCH_PATH // Search $PATH
                                        | Glib::SPAWN_DO_NOT_REAP_CHILD // Lets us do waitpid
                                    , sigc::slot<void>() // child setup
                                    , &m_pid
                                    , &m_infp // stdin
                                    );
    } catch (const Glib::Error &ex) {
        log_error() << "Failed to launch dbus-proxy";
        return false;
    }

    log_debug() << "Started dbus-proxy: " << m_pid;
    m_state = GatewayState::ACTIVATED;

    return true;
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

bool DBusGateway::teardownGateway()
{
    bool success = true;

    if (m_pid != INVALID_PID) {
        log_debug() << "Killing dbus-proxy with pid " << m_pid;

        kill(m_pid, SIGKILL); // In some configurations, hangs if using SIGTERM instead
        waitpid(m_pid, nullptr, 0); // Wait until it exits
        Glib::spawn_close_pid(m_pid);
    } else {
        log_debug() << "dbus-proxy pid not set or already dead: " << m_pid;
    }

    if (access(m_socket.c_str(), F_OK) != -1) {
        if (unlink(m_socket.c_str()) == -1) {
            log_error() << "Could not remove " << m_socket << ": " << strerror(errno);
            success = false;
        }
    } else {
        log_debug() << "Socket not accessible, has it been removed already?";
    }

    return success;
}

std::string DBusGateway::socketName()
{
    // Return the filename after stripping directory info
    std::string socket(m_socket.c_str());
    return socket.substr(socket.rfind('/') + 1);
}


