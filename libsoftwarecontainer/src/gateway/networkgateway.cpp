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


#include <cstring>
#include "ifaddrs.h"
#include "unistd.h"
#include "networkgateway.h"
#include "generators.h"
#include "networkgatewayparser.h"

NetworkGateway::NetworkGateway() :
    Gateway(ID),
    m_internetAccess(false),
    m_interfaceInitialized(false)
{
}

NetworkGateway::~NetworkGateway()
{
}

ReturnCode NetworkGateway::readConfigElement(const json_t *element)
{
    IPTableEntry e;
    NetworkGatewayParser configParser;
    ReturnCode returnValue = configParser.parseNetworkGatewayConfiguration(element, e);

    if (isSuccess(returnValue)) {
        m_entries.push_back(e);
    }

    // --- TEMPORARY WORKAROUND ---
    // in the wait of activate() being rewritten
    if (IPTableEntry::Target::ACCEPT == e.m_defaultTarget) {
        m_internetAccess = true;
        m_gateway = "10.0.3.1";
    }
    // ----------------------------
    return returnValue;
}




bool NetworkGateway::activateGateway()
{
    if (!hasContainer()) {
        log_error() << "activate was called on an EnvironmentGateway which has no associated container";
        return false;
    }

    if (m_gateway.size() != 0) {
        log_debug() << "Default gateway set to " << m_gateway;
    } else {
        m_internetAccess = false;
        log_debug() << "No gateway. Network access will be disabled";
    }

    if ( !isBridgeAvailable() ) {
        log_error() << "Bridge not available, expected gateway to be " << m_gateway;
        return false;
    }

    if (m_internetAccess) {
        generateIP();
        auto returnValue = up();

        for (auto entry : m_entries) {
            executeInContainer([&] () {
                if (isSuccess(entry.applyRules())) {
                    return 0;
                }  else {
                    return 1;
                }
            });
        }

        return returnValue;
    } else {
        return down();
    }
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

    m_ip = m_generator.genIPAddr(ipAddrNet);
    log_debug() << "IP set to " << m_ip;

    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    ReturnCode ret = executeInContainer([this] {
        Netlink n;
        ReturnCode success = n.setDefaultGateway(m_gateway.c_str());
        return isSuccess(success) ? 0 : 1;
    });

    return isSuccess(ret);
}

bool NetworkGateway::up()
{
    if (m_interfaceInitialized) {
        log_debug() << "Interface already configured";
        return true;
    }

    log_debug() << "Attempting to bring up eth0";
    ReturnCode ret = executeInContainer([this] {
        Netlink n;

        Netlink::LinkInfo iface;
        if (isError(n.findLink("eth0", iface))) {
            log_error() << "Could not find interface eth0 in container";
            return 1;
        }

        int ifaceIndex = iface.first.ifi_index;
        if (isError(n.linkUp(ifaceIndex))) {
            log_error() << "Could not bring interface eth0 up in container";
            return 2;
        }

        in_addr ip_addr;
        inet_aton(ip().c_str(), &ip_addr);
        return isSuccess(n.setIP(ifaceIndex, ip_addr, 24)) ? 0 : 3;
    });

    if (isSuccess(ret)) {
        m_interfaceInitialized = true;
        return setDefaultGateway();
    } else {
        log_debug() << "Failed to bring up eth0";
        return false;
    }
}

bool NetworkGateway::down()
{
    log_debug() << "Attempting to configure eth0 to 'down state'";
    ReturnCode ret = executeInContainer([this] {
        Netlink n;
        Netlink::LinkInfo iface;
        if (isError(n.findLink("eth0", iface))) {
            log_error() << "Could not find interface eth0 in container";
            return 1;
        }

        if (isError(n.linkDown(iface.first.ifi_index))) {
            log_error() << "Could not bring interface eth0 down in container";
            return 2;
        }

        return 0;
    });

    if (isError(ret)) {
        log_error() << "Configuring eth0 to 'down state' failed.";
        return false;
    }

    return true;
}

bool NetworkGateway::isBridgeAvailable()
{
    Netlink::LinkInfo iface;
    if (isError(m_netlinkHost.findLink(BRIDGE_DEVICE, iface))) {
        log_error() << "Could not find " << BRIDGE_DEVICE << " in the host";
    }

    std::vector<Netlink::AddressInfo> addresses;
    if (isError(m_netlinkHost.findAddresses(iface.first.ifi_index, addresses))) {
        log_error() << "Could not fetch addresses for " << BRIDGE_DEVICE << " in the host";
    }

    return isSuccess(m_netlinkHost.hasAddress(addresses, AF_INET, m_gateway.c_str()));
}
