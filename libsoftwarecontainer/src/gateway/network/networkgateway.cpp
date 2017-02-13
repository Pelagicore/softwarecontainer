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

#include <cstring>
#include "ifaddrs.h"
#include "unistd.h"
#include "networkgateway.h"
#include "networkgatewayparser.h"
#include "functionjob.h"

namespace softwarecontainer {

NetworkGateway::NetworkGateway(const int32_t id,
                               const std::string bridgeDevice,
                               const std::string gateway,
                               const uint8_t maskBits,
                               std::shared_ptr<ContainerAbstractInterface> container) :
    Gateway(ID, container),
    m_netmask(maskBits),
    m_gateway(gateway),
    m_bridgeDevice(bridgeDevice),
    m_interfaceInitialized(false),
    m_containerID(id)
{
}

NetworkGateway::~NetworkGateway() { }

bool NetworkGateway::readConfigElement(const json_t *element)
{
    IPTableEntry e;
    NetworkGatewayParser configParser;

    if (configParser.parseNetworkGatewayConfiguration(element, e)) {
        m_entries.push_back(e);
        return true;
    } else {
        return false;
    }
}

bool NetworkGateway::activateGateway()
{
    if (m_gateway.size() != 0) {
        log_debug() << "Default gateway set to " << m_gateway;
    } else {
        log_warning() << "No gateway. Network access will be disabled";
        return true;
    }

    if (!isBridgeAvailable()) {
        log_error() << "Bridge not available, expected gateway to be " << m_gateway;
        return false;
    }

    try {
        m_ip.s_addr = m_functions.generateIP(m_netmask, m_gateway, m_containerID);
    } catch (IPAllocationError &error) {
        log_error() << error.what();
        return false;
    }

    bool returnValue = up();
    if (!returnValue) {
        log_error() << "Couldn't bring the network up";
        return false;
    }

    log_debug() << "Adding iptables entries";
    for (auto entry : m_entries) {
        FunctionJob job (getContainer(), [&] () {
            return entry.applyRules() ? SUCCESS : FAILURE;
        });
        job.start();

        job.wait();
        if (job.isError()) {
            log_error() << "Failed to apply rules for entry: " << entry.toString();
            return false;
        }
    }

    return true;
}

bool NetworkGateway::teardownGateway()
{
    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    FunctionJob job(getContainer(), [this] {
        Netlink n;
        return n.setDefaultGateway(m_gateway.c_str()) ? SUCCESS : FAILURE;
    });

    job.start();
    job.wait();
    return job.isSuccess();
}

bool NetworkGateway::up()
{
    static const constexpr int BAD_SETIP = 3;

    if (m_interfaceInitialized) {
        log_debug() << "Interface already configured";
        return true;
    }

    log_debug() << "Attempting to bring up eth0";
    FunctionJob jobBringUpEthernet(getContainer(), [this] {
        Netlink n;

        Netlink::LinkInfo iface;
        if (!n.findLink("eth0", iface)) {
            return NO_LINK;
        }

        int ifaceIndex = iface.first.ifi_index;
        if (!n.linkUp(ifaceIndex)) {
            return BAD_LINKUP;
        }

        if (!n.setIP(ifaceIndex, m_ip, m_netmask)) {
            return BAD_SETIP;
        }

        return SUCCESS;
    });

    jobBringUpEthernet.start();

    int returnCode = jobBringUpEthernet.wait();
    switch(returnCode) {
        case NO_LINK:
            log_error() << "Could not find interface eth0 in container";
            return false;
        case BAD_LINKUP:
            log_error() << "Could not bring interface eth0 up in container";
            return false;
        case BAD_SETIP:
            log_error() << "Could not set IP-address";
            return false;
        case SUCCESS:
            log_debug() << "Interface brought up, proceeding to set default gateway";
            m_interfaceInitialized = true;
            return setDefaultGateway();
        default:
            log_error() << "Unhandled case in NetworkGateway::up(), this is an error!";
            return false;
    }
}

bool NetworkGateway::down()
{
    log_debug() << "Attempting to configure eth0 to 'down state'";
    FunctionJob job(getContainer(), [this] {
        Netlink n;
        Netlink::LinkInfo iface;
        if (!n.findLink("eth0", iface)) {
            return NO_LINK;
        }

        if (!n.linkDown(iface.first.ifi_index)) {
            return BAD_LINKDOWN;
        }

        return SUCCESS;
    });
    job.start();
    int returnCode = job.wait();
    switch(returnCode)
    {
        case NO_LINK:
            log_error() << "Could not find interface eth0 in container";
            return false;
        case BAD_LINKDOWN:
            log_error() << "Could not bring interface eth0 down in container";
            return false;
        case SUCCESS:
            return true;
        default:
            log_error() << "Unhandled case in NetworkGateway::down(), this is an error!";
            return false;
    }
}

bool NetworkGateway::isBridgeAvailable()
{
    log_debug() << "Is bridge available?";
    Netlink::LinkInfo iface;
    if (!m_netlinkHost.findLink(m_bridgeDevice.c_str(), iface)) {
        log_error() << "Could not find " << m_bridgeDevice << " in the host";
    }

    std::vector<Netlink::AddressInfo> addresses;
    if (!m_netlinkHost.findAddresses(iface.first.ifi_index, addresses)) {
        log_error() << "Could not fetch addresses for " << m_bridgeDevice << " in the host";
    }

    log_debug() << "Could find bridge and could fetch address, running hasAddress";
    bool retval = m_netlinkHost.hasAddress(addresses, AF_INET, m_gateway.c_str());
    return retval;
}

} // namespace softwarecontainer
