
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
#include "jansson.h"
#include "netlink.h"


/**
 * @brief A rules entry for the treatment of packets.
 */
struct IPTableEntry
{
    LOG_DECLARE_CLASS_CONTEXT("IPTE", "IPTable Entry");

    /**
     * @brief container for port filtering options. Used internally in a Rule.
     */
    struct portFilter {
        portFilter() : any{0}, multiport{0}, ports{""} {};
        bool any;
        bool multiport;
        std::string ports;
    };


    /**
     * @brief Definition of a 'Rule' used to handle network traffic. Used internally in an Entry.
     */
    struct Rule
    {
        std::string host;
        portFilter  ports;
        std::string target;
    };

    /**
     * @brief Inserts a rule to iptables
     * @return true  Upon success
     * @return false Upon failure
     */
    bool insertRule(Rule rule, std::string type);

    /**
     * @brief converts NetworkGateway type to rule table chain
     * Rule table consists three chains: "INPUT", "OUTPUT" and "FORWARD" which are activated at
     * different points of the packet filtering process
     * @return "INPUT", "OUTPUT" or "FORWARD" on successfully determine the chain
     * @return "" on failure to determine chain
     */
    std::string getChain();

    /**
     * @brief Applies all rules to iptables
     * @return true  Upon success
     * @return false Upon failure
     */
    bool applyRules();

    unsigned int m_priority;
    std::string m_type;
    std::vector<Rule> m_rules;
    std::string m_defaultTarget;
};


/**
 * @brief Sets up and manages network access and routing to the container
 *
 * The responsibility of NetworkGateway is to setup network connection as specified by given
 * configuration. This configuration is described in detail in the user documentation, but
 * a short summary of it is how to handle incoming and outgoing network packages using the
 * three targets: ACCEPT, DROP and REJECT.
 */
class NetworkGateway :
        public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("NETG", "Network gateway");

public:
    static constexpr const char *ID = "network";

    NetworkGateway();
    ~NetworkGateway();

    ReturnCode readConfigElement(const json_t *element) override;

    /**
     * @brief Implements Gateway::activateGateway
     */
    bool activateGateway() override;

    /**
     * @brief Implements Gateway::teardownGateway
     */
    bool teardownGateway() override;

    /**
     * @brief Returns the IP of the container
     */
    const std::string ip();
private:
    /**
     * @brief Generate IP address for the container
     *
     * Retrieves an IP from DHCP.
     *
     * Note that a file on the system acts as a placeholder for the DHCP server.
     * The file keeps track of the highest used IP address.
     *
     * @return true  Upon success
     * @return false Upon failure
     */
    bool generateIP();

    /**
     * @brief Set route to default gateway
     *
     * Sets the route to the default gateway.
     * To be able to access anything outside the container, this method must be
     * called after the network interface has been enabled. This is also true for
     * cases when a network interface that was previously enabled has been disabled
     * and then enabled again.
     *
     * @return true  Upon success
     * @return false Upon failure
     */
    bool setDefaultGateway();

    /**
     * @brief Enable the default network interface
     *
     * Enables the network interface and calls NetworkGateway::setDefaultGateway().
     *
     * When this is done for the first time, i.e. during the first call to activate()
     * the IP and netmask are also set. During subsequent calls, this merely brings
     * up the existing network interface and calls setDefaultGateway().
     *
     * @return true  Upon success
     * @return false Upon failure
     */
    bool up();

    /**
     * @brief Disable the default network interface
     *
     * Disables the network interface.
     *
     * @return true  Upon success
     * @return false Upon failure
     */
    bool down();

    /**
     * @brief Check the availability of the network bridge on the host
     *
     * Checks the availability of the required network bridge on the host.
     *
     * @return true  If bridge interface is available
     * @return false If bridge interface is not available
     */
    virtual bool isBridgeAvailable();

    std::string m_ip;
    std::string m_gateway;

    std::vector<IPTableEntry> m_entries;

    bool m_internetAccess;
    bool m_interfaceInitialized;

    Generator m_generator;
    Netlink m_netlinkHost;

    /**
     * @brief Parses a json element to a Rule
     * @return ReturnCode::SUCCESS if the rule is successfully parsed
     * @return ReturnCode::FAILURE otherwise.
     */
    virtual ReturnCode parseRule(const json_t *element, std::vector<IPTableEntry::Rule> &rules);

    /**
     * @brief Parses a ports from an json element
     * @return ReturnCode::SUCCESS if the rule is successfully parsed
     * @return ReturnCode::FAILURE otherwise.
     */
    virtual ReturnCode parsePort(const json_t *element, IPTableEntry::portFilter &ports);

    /**
     * @brief Parses a string to a Target
     * @return Either the valid Target representation of true or false
     */
    bool parseTarget(const std::string &str);
};
