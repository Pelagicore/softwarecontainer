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

#pragma once

#include "gateway/gateway.h"

namespace softwarecontainer {

/**
 * @brief DBus Gateway takes care of spawning and killing the DBus proxies.
 *
 *  This module requires the 'dbus-proxy' binary to be available in the $PATH
 *  of the user executing softwarecontainer.
 */
class DBusGatewayInstance :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("DBSI", "DBus gateway instance");

public:
    enum ProxyType { SessionProxy, SystemProxy };

    static constexpr const char *ID = "dbus-bus-instance";

    static constexpr const char *SESSION_CONFIG = "dbus-gateway-config-session";
    static constexpr const char *SYSTEM_CONFIG = "dbus-gateway-config-system";

    /**
     * @brief Spawn the proxy and use the supplied path for the socket
     *
     * @param type SessionProxy or SystemProxy
     */
    DBusGatewayInstance(ProxyType type,
                        const std::string &gatewayDir,
                        std::shared_ptr<ContainerAbstractInterface> container);
    virtual ~DBusGatewayInstance();

    virtual bool readConfigElement(const json_t *element) override;

    /**
     *  @brief Implements Gateway::activateGateway
     *
     *  Starts the dbus-proxy binary and feeds it the configuration set in
     *  setConfig(). This function will also set up the correct environment
     *  variables (DBUS_SESSION_BUS_ADDRESS when proxying the session bus, or
     *  DBUS_SYSTEM_BUS_ADDRESS when proxying the system bus).
     *
     *  This function requires a config to have been set using setConfig().
     *
     *  This function requires the 'dbus-proxy' binary to be available in $PATH
     *
     *  @return true when dbus-proxy was correctly execute and environment
     *   variable was correctly set
     *   false if dbus-proxy failed to execute or accept input on STDIN,
     *   or when environment variable could not be set.
     */
    virtual bool activateGateway();

    /**
     * @brief Implements Gateway::teardownGateway
     *
     *  This function will clean up processes launched and file descriptors
     *  opened during the lifetime of the gatway. Specifically it will close
     *  the connection to the dbus-proxy, and close the stdin and stdout pipes
     *  opened to dbus-proxy.
     *
     * @return false if dbus-proxy could not be terminated, if stdin or stdout
     *          could not be closed, or if the socket created by the dbus-proxy
     *          could not be removed.
     */
    virtual bool teardownGateway() override;

private:

    // JSON stuff. These are members to allow continous appends to the config
    json_t *m_entireConfig;
    json_t *m_busConfig;
    const char* typeStr;

    std::string socketName();
    bool isSocketCreated() const;

    // Socket used for exposing D-Bus in container
    std::string m_socket;

    // Session or system, depending on the type of gateway being started
    ProxyType m_type;

    // pid of dbus-proxy instance
    pid_t m_pid = INVALID_PID;

    // STDIN for dbus-proxy instance
    int m_proxyStdin = INVALID_FD;

    virtual bool startDBusProxy(const std::vector<std::string> &commandVec, const std::vector<std::string> &envVec);

    /**
     * @brief This function will send configuration to dbus-proxy. The configuration here indicates
     * a list off all configuration parameters that set. The dbus-proxy uses configuration as a
     * filter list.
     *
     * When a D-Bus message comes to dbus-proxy, it is allowed only if there is a configuration with
     * all matched parameters direction, interface, path and method.
     *
     * @return true if the configuration is successfully passed to dbus-proxy.
     *         false otherwise
     */
    virtual bool testDBusConnection(const std::string &config);
};

} // namespace softwarecontainer
