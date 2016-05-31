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
    ReturnCode returnCode = ReturnCode::SUCCESS;

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
            returnCode = ReturnCode::FAILURE;
        }
    }

    return returnCode;
}

bool NetworkGateway::activate()
{
    if (m_gateway.size() != 0) {
        log_debug() << "Default gateway set to " << m_gateway;
    } else {
        m_internetAccess = false;
        log_debug() << "No gateway. Network access will be disabled";
    }

    bool success = false;
    bool ready = false;

    if (isBridgeAvailable()) {
        if (generateIP()) {
            ready = true;
        }
    }

    if (ready) {
        if (m_internetAccess) {
            success = up();
        } else {
            success = down();
        }
    }

    return success;
}

const std::string NetworkGateway::ip()
{
    return m_ip;
}

bool NetworkGateway::generateIP()
{
    const char *ipAddrNet = m_gateway.substr(0, m_gateway.size() - 1).c_str();

    m_ip = m_generator.gen_ip_addr(ipAddrNet);
    log_debug() << "IP set to " << m_ip;

    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    std::string cmd = "route add default gw " + m_gateway;
    return isSuccess(executeInContainer(cmd));
}

bool NetworkGateway::up()
{
    std::string cmd;

    if (!m_interfaceInitialized) {
        cmd = "ifconfig eth0 " + m_ip + " netmask 255.255.255.0 up";
        m_interfaceInitialized = true;
    } else {
        cmd = "ifconfig eth0 up";
    }

    if (isError(executeInContainer(cmd))) {
        return false;
    }

    /* The route to the default gateway must be added
       each time the interface is brought up again */
    return setDefaultGateway();
}

bool NetworkGateway::down()
{
    std::string cmd = "ifconfig eth0 down";
    return isSuccess(executeInContainer(cmd));
}


bool NetworkGateway::isBridgeAvailable()
{
    bool ret = false;
    std::string cmd = StringBuilder() << "ifconfig | grep -C 2 \"" << BRIDGE_INTERFACE << "\" | grep -q \"" << m_gateway << "\"";

    if (isSuccess(executeInContainer(cmd))) {
        ret = true;
    } else {
        log_error() << "No network bridge configured";
    }

    return ret;
}
