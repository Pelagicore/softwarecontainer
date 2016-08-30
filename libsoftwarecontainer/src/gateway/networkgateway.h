
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


#pragma once

#include "gateway.h"
#include "generators.h"

class NetworkGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("NETG", "Network gateway");

public:
    static constexpr const char *ID = "network";

    static constexpr const char *BRIDGE_INTERFACE = "lxcbr0";

    NetworkGateway();
    ~NetworkGateway();

    ReturnCode readConfigElement(const json_t *element) override;

    /*!
     *  Implements Gateway::activateGateway
     */
    bool activateGateway() override;

    /*!
     * Implements Gateway::teardownGateway
     */
    bool teardownGateway() override;

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
    virtual bool isBridgeAvailable();

    std::string m_ip;
    std::string m_gateway;
    bool m_internetAccess;
    bool m_interfaceInitialized;

    Generator m_generator;
};
