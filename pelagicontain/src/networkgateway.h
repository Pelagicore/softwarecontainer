/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef NETWORKGATEWAY_H
#define NETWORKGATEWAY_H

#include "gateway.h"
#include "systemcallinterface.h"
#include "generators.h"

class NetworkGateway : public Gateway {
    LOG_DECLARE_CLASS_CONTEXT("NETG", "Network gateway");

public:
    NetworkGateway(SystemcallAbstractInterface &systemCallInterface);
    ~NetworkGateway();

    /*!
     *  Implements Gateway::id
     */
    virtual std::string id();

    ReturnCode readConfigElement(JSonElement &element) override;

    /*!
     *  Implements Gateway::activate
     */
    virtual bool activate();

    /*! Returns the IP of the container
     */
    const std::string ip();
private:
    /*! Generate IP address for the container
     *
     * Retrieves an IP from DHCP.
     *
     * Note that a file on the system acts as a placeholder for the DHCP server.
     * The file keeps track of the highest used IP address.
     *
     * \return true  Upon success
     * \return false Upon failure
     */
    bool generateIP();

    /*! Set route to default gateway
     *
     * Sets the route to the default gateway.
     * To be able to access anything outside the container, this method must be
     * called after the network interface has been enabled. This is also true for
     * cases when a network interface that was previously enabled has been disabled
     * and then enabled again.
     *
     * \return true  Upon success
     * \return false Upon failure
     */
    bool setDefaultGateway();

    /*! Enable the default network interface
     *
     * Enables the network interface and calls NetworkGateway::setDefaultGateway().
     *
     * When this is done for the first time, i.e. during the first call to activate()
     * the IP and netmask are also set. During subsequent calls, this merely brings
     * up the existing network interface and calls setDefaultGateway().
     *
     * \return true  Upon success
     * \return false Upon failure
     */
    bool up();

    /*! Disable the default network interface
     *
     * Disables the network interface.
     *
     * \return true  Upon success
     * \return false Upon failure
     */
    bool down();

    /*! Check the availability of the network bridge on the host
     *
     * Checks the availability of the required network bridge on the host.
     *
     * \return true  If bridge interface is available
     * \return false If bridge interface is not available
     */
    bool isBridgeAvailable();

    /*! Parse the JSON configuration passed down from Platform Access Manager
     *
     *  Returns the string value of the "gateway" key. If there is an error
     *  or the value is an empty string, an empty string is returned.
     *
     * \param config The JSON string containing the configuration
     * \return An empty string if invalid or empty key, otherwise string with
     *         gateway
     */
    std::string gatewayFromConfig(const std::string &config);

    std::string m_ip;
    std::string m_gateway;
    bool m_internetAccess;
    bool m_interfaceInitialized;
    SystemcallAbstractInterface &m_systemCallInterface;

    Generator m_generator;

};

#endif /* NETWORKGATEWAY_H */
