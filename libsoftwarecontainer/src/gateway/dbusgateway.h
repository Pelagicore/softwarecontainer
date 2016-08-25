
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


#ifndef DBUSGATEWAY_H
#define DBUSGATEWAY_H

#include "gateway.h"

/*! DBus Gateway takes care of spawning and killing the DBus proxies.
 *  =================================================================
 *
 *  This module requires the 'dbus-proxy' binary to be available in the $PATH
 *  of the user executing softwarecontainer.
 *
 *  The gateway will create a socket for the D-Bus connection being proxied.
 *  The socket will be placed in a directory accessible from within the
 *  container, and the applications running inside the container are expected
 *  to use this socket when communicating with the outside D-Bus system.
 *
 *  The gateway will also set the DBUS_SESSION_BUS_ADDRESS or
 *  DBUS_SYSTEM_BUS_ADDRESS for the session and system bus, respectively.
 *  libdbus uses these variables to find the socket to use for D-Bus
 *  communication, and the application running withing the container is
 *  expected to use these variables (probably by using libdbus)
 *
 *  The gateway will not do any analysis of the configuration passed
 *  to it through setConfig(), but will pass this configuration along to
 *  dbus-proxy verbatim. This is to support future changes in the configuration
 *  format.
 *
 *  Configuration Content and Structure
 *  ===================================
 *
 *  The session and system buses have separate configurations implemented as separate JSON array
 *  objects with the names:
 *  * dbus-gateway-config-session
 *  * dbus-gateway-config-system
 *
 *  The arrays contain JSON objects where each object is an access rule specified as a combination
 *  of:
 *  * Direction of method call or signal, i.e. if the call or signal is outgoing from inside the
 *    container or incoming from the outside of the container.
 *  * D-Bus interface of the method or signal.
 *  * Object path where the interface is implemented.
 *  * The method or signal name the rule is for.
 *
 *  The rules are implemented as name/value pairs:
 *  * ''direction'' - A string set to either "incoming" if the call or signal is coming from the
 *    outside of the container, or "outgoing" if the call or signal is coming from inside of the
 *    container.
 *  * ''interface'' - A string specifying a D-Bus interface name, e.g."org.freedesktop.DBus".
 *  * ''object-path'' - A string specifying a D-Bus object path, e.g. "/org/freedesktop/UPower/Policy".
 *  * ''method'' - A string specifying a D-Bus method name or signal name, e.g. "EnumerateDevices".
 *
 *  All the values can be substituted with the wildcard character "*" with the meaning "all", e.g. a
 *  "direction" set to "*" will mean both incoming and outgoing, and a "method" set to "*" will match
 *  all method and signal names for the interface and object path specified. If a bus configuration
 *  is just an empty array it means all access to that bus will be blocked.
 *
 *  Example Configurations
 *  ======================
 *
 *  A configuration that provides full access to the system and session buses would look like:
 *
 *  \code{json}
 *  {
 *      "dbus-gateway-config-session": [
 *          {
 *              "direction": "*",
 *              "interface": "*",
 *              "object-path": "*",
 *              "method": "*"
 *          }
 *      ],
 *      "dbus-gateway-config-system": [
 *          {
 *              "direction": "*",
 *              "interface": "*",
 *              "object-path": "*",
 *              "method": "*"
 *          }
 *      ]
 *  }
 *  \endcode
 *
 *  A configuration that provides full access to the session bus and no access at all to the system
 *  bus would look like:
 *
 *  \code{json}
 *  {
 *      "dbus-gateway-config-session": [
 *          {
 *              "direction": "*",
 *              "interface": "*",
 *              "object-path": "*",
 *              "method": "*"
 *          }
 *      ],
 *      "dbus-gateway-config-system": []
 *  }
 *  \endcode
 *
 *  A configuration that allows introspection on the session bus from within the container and no
 *  access at all to the system bus would look like:
 *
 *  \code{json}
 *  {
 *      "dbus-gateway-config-session": [
 *          {
 *              "direction": "outgoing",
 *              "interface": "org.freedesktop.DBus.Introspectable",
 *              "object-path": "/",
 *              "method": "Introspect"
 *          }
 *      ],
 *      "dbus-gateway-config-system": []
 *  }
 *  \endcode
 */
class DBusGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("DBUS", "DBus gateway");

public:
    enum ProxyType { SessionProxy, SystemProxy };

    static constexpr const char *ID = "dbus";

    /*! Spawn the proxy and use the supplied path for the socket
     *
     * \param  type       SessionProxy or SystemProxy
     */
    DBusGateway(ProxyType type, const std::string &gatewayDir, const std::string &name);
    virtual ~DBusGateway();

    virtual ReturnCode readConfigElement(const JSonElement &element) override;

    /*!
     *  Implements Gateway::activateGateway
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
     *  \return true when dbus-proxy was correctly execute and environment
     *          variable was correctly set
     *  \return false if dbus-proxy failed to execute or accept input on STDIN,
     *          or when environment variable could not be set.
     */
    virtual bool activateGateway();

    /*! Implements Gateway::teardownGateway
     *
     *  This function will clean up processes launched and file descriptors
     *  opened during the lifetime of the gatway. Specifically it will close
     *  the connection to the dbus-proxy, and close the stdin and stdout pipes
     *  opened to dbus-proxy.
     *
     *  \return false if dbus-proxy could not be terminated, if stdin or stdout
     *          could not be closed, or if the socket created by the dbus-proxy
     *          could not be removed.
     */
    virtual bool teardownGateway() override;

private:
    std::string socketName();
    bool isSocketCreated() const;

    // Socket used for exposing D-Bus in container
    std::string m_socket;

    // Session or system, depending on the type of gateway being started
    ProxyType m_type;

    json_t *m_sessionBusConfig;
    json_t *m_systemBusConfig;

    // pid of dbus-proxy instance
    pid_t m_pid = INVALID_PID;

    // STDIN for dbus-proxy instance
    int m_infp = INVALID_FD;

    virtual bool startDBusProxy(const std::vector<std::string> &commandVec, const std::vector<std::string> &envVec);
    virtual bool testDBusConnection(const std::string &config);
};

#endif /* DBUSGATEWAY_H */
