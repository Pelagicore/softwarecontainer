/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DBUSGATEWAY_H
#define DBUSGATEWAY_H

#include <string>
#include <unistd.h>
#include "gateway.h"

/*! DBus Gateway takes care of spawning and killing the DBus proxies.
 *
 *  This module requires the 'dbus-proxy' binary to be available in the $PATH
 *  of the user executing pelagicontain.
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
 *  The gateway will not do any thorough analysis of the configuration passed
 *  to it through setConfig(), but will pass this configuration along to
 *  dbus-proxy verbatim. This is to support future changes in the configuration
 *  format.
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
    ~DBusGateway();

    virtual ReturnCode readConfigElement(const JSonElement &element) override;

    /*!
     *  Implements Gateway::activate
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
    virtual bool activate();

    /*! Implements Gateway::teardown
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
    virtual bool teardown();

private:
    ReturnCode makePopenCall(const std::string &command, int &infp, int &outfp, pid_t &pid);

    bool makePcloseCall(pid_t pid, int infp, int outfp);

    const char *typeString();
    std::string socketName();
    bool isSocketCreated() const;

    // Socket used for exposing D-Bus in container
    std::string m_socket;

    // Session or system, depending on the type of gateway being started
    ProxyType m_type;

    std::string m_sessionBusConfig;
    std::string m_systemBusConfig;

    // pid of dbus-proxy instance
    pid_t m_pid = INVALID_PID;

    // STDIN and STDOUT for dbus-proxy instance
    int m_infp;
    int m_outfp;

    // Keeps track of whether setConfig() has been run or not
    bool m_hasBeenConfigured;

    // Keeps track of whether the dbus-proxy program has been started
    // this is used for deciding what teardown is necessary
    bool m_dbusProxyStarted;
};

#endif /* DBUSGATEWAY_H */
