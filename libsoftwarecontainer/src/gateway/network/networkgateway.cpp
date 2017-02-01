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
                               const std::string gateway,
                               const uint8_t maskBits) :
    Gateway(ID),
    m_netmask(maskBits),
    m_gateway(gateway),
    m_interfaceInitialized(false),
    m_containerID(id)
{
}

NetworkGateway::~NetworkGateway() { }

ReturnCode NetworkGateway::readConfigElement(const json_t *element)
{
    IPTableEntry e;
    NetworkGatewayParser configParser;
    ReturnCode returnValue = configParser.parseNetworkGatewayConfiguration(element, e);

    if (isSuccess(returnValue)) {
        m_entries.push_back(e);
    }

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
        log_debug() << "No gateway. Network access will be disabled";
    }

    if (isError(isBridgeAvailable())) {
        log_error() << "Bridge not available, expected gateway to be " << m_gateway;
        return false;
    }

    try {
        m_ip.s_addr = m_functions.generateIP(m_netmask, m_gateway, m_containerID);
    } catch (IPAllocationError &error) {
        log_error() << error.what();
        return false;
    }

    auto returnValue = up();

    for (auto entry : m_entries) {
        FunctionJob job (getContainer(), [&] () {
            if (isSuccess(entry.applyRules())) {
                return 0;
            }  else {
                return 1;
            }
        });
        job.start();

        job.wait();
        if (job.isError()) {
            log_debug() << "Failed to apply rules for entry: " << entry.toString();
        }

    }

    return returnValue;
}

bool NetworkGateway::teardownGateway()
{
    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    FunctionJob job(getContainer(), [this] {
        Netlink n;
        ReturnCode success = n.setDefaultGateway(m_gateway.c_str());
        return isSuccess(success) ? 0 : 1;
    });

    job.start();
    job.wait();
    return job.isSuccess();
}

bool NetworkGateway::up()
{
    if (m_interfaceInitialized) {
        log_debug() << "Interface already configured";
        return true;
    }

    log_debug() << "Attempting to bring up eth0";
    FunctionJob jobBringUpEthernet(getContainer(), [this] {
        Netlink n;

        Netlink::LinkInfo iface;
        if (isError(n.findLink("eth0", iface))) {
            return 1;
        }

        int ifaceIndex = iface.first.ifi_index;
        if (isError(n.linkUp(ifaceIndex))) {
            return 2;
        }

        return isSuccess(n.setIP(ifaceIndex, m_ip, 24)) ? 0 : 3;
    });

    jobBringUpEthernet.start();

    int returnCode = jobBringUpEthernet.wait();
    if (jobBringUpEthernet.isError()) {
        if (returnCode == 1) {
            log_error() << "Could not find interface eth0 in container";
            return false;
        }

        if (returnCode == 2) {
            log_error() << "Could not bring interface eth0 up in container";
            return false;
        }

        if (returnCode == 3) {
            log_error() << "Could not set IP-address";
            return false;
        }
    }

    m_interfaceInitialized = true;
    return setDefaultGateway();
}

ReturnCode NetworkGateway::down()
{
    log_debug() << "Attempting to configure eth0 to 'down state'";
    FunctionJob job(getContainer(), [this] {
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
    job.start();
    job.wait();
    if (job.isError()) {
        log_error() << "Configuring eth0 to 'down state' failed.";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

/*
 * TODO: Use value from the config instead of define here.
 */
ReturnCode NetworkGateway::isBridgeAvailable()
{
    Netlink::LinkInfo iface;
    if (isError(m_netlinkHost.findLink(SC_BRIDGE_DEVICE, iface))) {
        log_error() << "Could not find " << SC_BRIDGE_DEVICE << " in the host";
    }

    std::vector<Netlink::AddressInfo> addresses;
    if (isError(m_netlinkHost.findAddresses(iface.first.ifi_index, addresses))) {
        log_error() << "Could not fetch addresses for " << SC_BRIDGE_DEVICE << " in the host";
    }

    return m_netlinkHost.hasAddress(addresses, AF_INET, m_gateway.c_str());
}

} // namespace softwarecontainer
