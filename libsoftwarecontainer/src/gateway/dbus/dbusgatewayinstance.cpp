/*
 * Copyright (C) 2016-2017 Pelagicore AB
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

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "dbusgateway.h"
#include "dbusgatewayparser.h"

namespace softwarecontainer {

// These lines are needed in order to define the fields, which otherwise would
// yield linker errors.
constexpr const char *DBusGatewayInstance::SESSION_CONFIG;
constexpr const char *DBusGatewayInstance::SYSTEM_CONFIG;

DBusGatewayInstance::DBusGatewayInstance(ProxyType type,
                                         const std::string &gatewayDir,
                                         std::shared_ptr<ContainerAbstractInterface> container) :
    Gateway(ID, container, true /*this GW is dynamic*/),
    m_socket(""),
    m_type(type),
    m_pid(INVALID_PID),
    m_proxyStdin(INVALID_FD)
{
    std::string name = container->id();
    std::string socketName = (m_type == SessionProxy ? "sess_" : "sys_") + name + ".sock";
    m_socket = buildPath(gatewayDir, socketName);

    // Write configuration
    m_entireConfig = json_object();
    m_busConfig = json_array();

    const char* unusedTypeStr;
    if (m_type == SessionProxy) {
        typeStr = SESSION_CONFIG;
        unusedTypeStr = SYSTEM_CONFIG;

    } else {
        typeStr = SYSTEM_CONFIG;
        unusedTypeStr = SESSION_CONFIG;
    }
    json_object_set(m_entireConfig, typeStr, m_busConfig);

    // Send an empty array as config for the irrelevant proxy type
    json_t* emptyArray = json_array();
    json_object_set(m_entireConfig, unusedTypeStr, emptyArray);
}

DBusGatewayInstance::~DBusGatewayInstance()
{
    // TODO: The "extra" teardown here is likely unnecessary. This should be removed
    //       but is pending a decision to do explicit calls to teardown or let this
    //       happen in gateway destructors always. Currently this is not a problem,
    //       it's just cruft, but we don't want to touch it until there's time
    //       to make sure we do it correctly.
    if (isActivated()) {
        teardown();
    }
}

bool DBusGatewayInstance::readConfigElement(const json_t *element)
{
    DBusGatewayParser parser;

    // TODO: This should really be done with exceptions instead and json_t* as return type.
    if (!parser.parseDBusConfig(element, typeStr, m_busConfig)) {
        // This might also mean that only one of session or system bus config parsing failed
        // so this is not a fatal error
        log_warning() << "Failed to parse DBus configuration element";
        return false;
    }

    return true;
}

bool DBusGatewayInstance::activateGateway()
{
    // Setting up the environment and starting dbus-proxy should only be done the first time
    // around. Any subsequent calls after the first should only result in the config being
    // updated.
    if (!m_activatedOnce) {
        // set DBUS_{SESSION,SYSTEM}_BUS_ADDRESS env variable
        std::string variable = std::string("DBUS_")
                             + (m_type == SessionProxy ? "SESSION" : "SYSTEM")
                             + std::string("_BUS_ADDRESS");
        std::string value = "unix:path=" + buildPath("/gateways", socketName());
        getContainer()->setEnvironmentVariable(variable, value);

        std::vector<std::string> commandVec = {"dbus-proxy",
                                               m_socket,
                                               m_type == SessionProxy ? "session" : "system"};

        // Give the dbus-proxy access to the real dbus bus address.
        std::vector<std::string> envVec;

        bool hasEnvVar = false;
        std::string envValue = Glib::getenv(variable, hasEnvVar);

        if (!hasEnvVar) {
            if (SessionProxy == m_type) {
                log_error() << "Using DBus gateway in session mode"
                            << " and no " + variable + " set in host environment, dbus-proxy won't work";
                return false;
            } else {
                log_warn() << "Using DBus gateway in system mode"
                           << " and no " + variable + " set in host environment, this could be a problem";
            }
        } else {
            envVec.push_back(variable + "=" + envValue);
        }

        if (!startDBusProxy(commandVec, envVec)) {
            return false;
        }
    }

    // Dump to string. Compact format is currently needed as dbus-proxy relies on
    // the config string to be without newlines anywhere in the middle of it all
    char *config_c = json_dumps(m_entireConfig, JSON_COMPACT);
    std::string config = std::string(config_c);

    free(config_c);

    return testDBusConnection(config);
}

bool DBusGatewayInstance::testDBusConnection(const std::string &configuration)
{
    // The reading end expects there to be one newline at the end
    std::string config{configuration + std::string("\n")};

    ssize_t count = config.length() * sizeof(char);
    log_debug() << "Expected config byte length " << count;

    log_debug() << "Config: " << config;

    ssize_t configWrite = write(m_proxyStdin, config.c_str(), count);

    // writing didn't work at all
    if (-1 == configWrite) {
        log_error() << "Failed to write to STDIN of dbus-proxy: " << strerror(errno);
        return false;
    } else if (configWrite == (ssize_t)count) {
        log_debug() << "Wrote " << configWrite << " bytes to dbus-proxy";
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

bool DBusGatewayInstance::startDBusProxy(const std::vector<std::string> &commandVec,
                                         const std::vector<std::string> &envVec)
{
    // Spawn dbus-proxy with access to its stdin.
    try {
        Glib::spawn_async_with_pipes(".",
                                     commandVec,
                                     envVec,
                                     Glib::SPAWN_STDOUT_TO_DEV_NULL // Redirect stdout
                                        | Glib::SPAWN_STDERR_TO_DEV_NULL // Redirect stderr
                                        | Glib::SPAWN_SEARCH_PATH // Search $PATH
                                        | Glib::SPAWN_DO_NOT_REAP_CHILD, // Lets us do waitpid
                                     sigc::slot<void>(), // child setup
                                     &m_pid,
                                     &m_proxyStdin);
    } catch (const Glib::Error &ex) {
        log_error() << "Failed to launch dbus-proxy";
        return false;
    }

    m_activatedOnce = true;
    log_debug() << "Started dbus-proxy: " << m_pid;

    return true;
}

bool DBusGatewayInstance::isSocketCreated() const
{
    int maxCount = 1000;
    int count = 0;
    do {
        if (count >= maxCount) {
            log_error() << "Could not find dbus-proxy socket, error: " << strerror(errno);
            return false;
        }
        count++;
        usleep(1000 * 10);
    } while (access(m_socket.c_str(), F_OK) == -1);
    return true;
}

bool DBusGatewayInstance::teardownGateway()
{
    bool success = true;

    if (nullptr != m_entireConfig) {
        json_decref(m_entireConfig);
        m_entireConfig = nullptr;
    }

    if (m_pid != INVALID_PID) {
        log_debug() << "Killing dbus-proxy with pid " << m_pid;

        // TODO: Figure out how to shut down nicely?
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
        // TODO: Seems weird that this would ever happen. Seems like a severe error.
        log_debug() << "Socket not accessible, has it been removed already?";
    }

    // Close stdin to proxy as we will no longer send configs to it
    if (close(m_proxyStdin) == -1) {
        log_warning() << "Could not close stdin of dbus-proxy";
    }

    return success;
}

std::string DBusGatewayInstance::socketName()
{
    return basename(m_socket.c_str());
}

} // namespace softwarecontainer
