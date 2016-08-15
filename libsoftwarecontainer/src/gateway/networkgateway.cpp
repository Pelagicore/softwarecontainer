/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <cstring>
#include "ifaddrs.h"
#include "unistd.h"
#include "networkgateway.h"
#include "jansson.h"
#include "generators.h"

constexpr const char *NetworkGateway::BRIDGE_INTERFACE;

NetworkGateway::NetworkGateway() :
    Gateway(ID),
    m_internetAccess(false),
    m_interfaceInitialized(false)
{
}

NetworkGateway::~NetworkGateway()
{
}

ReturnCode NetworkGateway::readConfigElement(const JSonElement &element)
{
    bool enableInternetAccess = false;
    element.read("internet-access", enableInternetAccess);
    m_internetAccess |= enableInternetAccess;

    log_debug() << (m_internetAccess ? "Internet access will be enabled" : "Internet access disabled");

    std::string gateway;
    element.read("gateway", gateway);
    if (gateway.size() != 0) {
        m_gateway = gateway;
        if ((m_gateway.size() != 0) && (m_gateway.compare(gateway))) {
            log_error() << "Contradiction in gateway";
            return ReturnCode::FAILURE;
        }
    }

    return ReturnCode::SUCCESS;
}

bool NetworkGateway::activateGateway()
{
    if (m_gateway.size() != 0) {
        log_debug() << "Default gateway set to " << m_gateway;
    } else {
        m_internetAccess = false;
        log_debug() << "No gateway. Network access will be disabled";
    }

    bool success = false;
    if (isBridgeAvailable()) {
        if (m_internetAccess) {
            generateIP();
            success = up();
        } else {
            success = down();
        }
    }
    return success;
}

bool NetworkGateway::teardownGateway()
{
    return true;
}

const std::string NetworkGateway::ip()
{
    return m_ip;
}

bool NetworkGateway::generateIP()
{
    log_debug() << "Generating ip-address";
    const char *ipAddrNet = m_gateway.substr(0, m_gateway.size() - 1).c_str();

    m_ip = m_generator.gen_ip_addr(ipAddrNet);
    log_debug() << "IP set to " << m_ip;

    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    log_debug() << "Attempting to set default gateway";
    std::string cmd = "route add default gw " + m_gateway;
    if (isError(executeInContainer(cmd))) {
        log_error() << "Could not set default gateway.";
        return false;
    }
    return true;
}

bool NetworkGateway::up()
{
    log_debug() << "Attempting to configure eth0 to 'up state'";
    std::string cmd;

    if (!m_interfaceInitialized) {
        cmd = "ifconfig eth0 " + m_ip + " netmask 255.255.255.0 up";
        m_interfaceInitialized = true;
    } else {
        cmd = "ifconfig eth0 up";
    }

    if (isError(executeInContainer(cmd))) {
        log_error() << "Configuring eth0 to 'up state' failed.";
        return false;
    }

    /* The route to the default gateway must be added
       each time the interface is brought up again */
    return setDefaultGateway();
}

bool NetworkGateway::down()
{
    log_debug() << "Attempting to configure eth0 to 'down state'";
    std::string cmd = "ifconfig eth0 down";
    if (isError(executeInContainer(cmd))) {
        log_error() << "Configuring eth0 to 'down state' failed.";
        return false;
    }
    return true;
}


bool NetworkGateway::isBridgeAvailable()
{
    log_debug() << "Checking bridge availability";
    std::string cmd = StringBuilder() << "ifconfig | grep -C 2 \""
                                      << BRIDGE_INTERFACE << "\" | grep -q \""
                                      << m_gateway << "\"";

    if (system(cmd.c_str()) == 0) {
        return true;
    } else {
        log_error() << "No network bridge configured";
    }

    return false;
}
