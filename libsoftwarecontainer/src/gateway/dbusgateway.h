
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

/*!
 * DBus Gateway takes care of spawning and killing the DBus proxies.
 *  =================================================================
 *
 *  This module requires the 'dbus-proxy' binary to be available in the $PATH
 *  of the user executing softwarecontainer.
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

    virtual ReturnCode readConfigElement(const json_t *element) override;

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
