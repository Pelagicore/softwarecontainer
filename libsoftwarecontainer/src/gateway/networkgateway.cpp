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
    if (!read(element, "type", e.m_type)) {
        log_error() << "No type specified in network config.";
        return ReturnCode::FAILURE;
    }

    if (e.m_type != "INCOMING" && e.m_type != "OUTGOING") {
        log_error() << e.m_type << " is not a valid type ('INCOMING' or 'OUTGOING')";
        return ReturnCode::FAILURE;
    }

    int p;
    if (!read(element, "priority", p)) {
        log_error() << "No priority specified in network config.";
        return ReturnCode::FAILURE;
    }
    e.m_priority = p;

    if (e.m_priority < 1) {
        log_error() << "Priority can not be less than 1 but is " << e.m_priority;
        return ReturnCode::FAILURE;
    }

    const json_t *rules = json_object_get(element, "rules");

    if (rules == nullptr) {
        log_error() << "No rules specified";
        return ReturnCode::FAILURE;
    }

    if (!json_is_array(rules)) {
        log_error() << "Rules not specified as an array";
        return ReturnCode::FAILURE;
    }

    size_t ix;
    json_t *val;
    json_array_foreach(rules, ix, val) {
        if (json_is_object(val)) {
            if (isError(parseRule(val, e.m_rules))) {
                log_error() << "Could not parse rule config";
                return ReturnCode::FAILURE;
            }
        } else {
            log_error() << "formatting of rules array is incorrect.";
            return ReturnCode::FAILURE;
        }
    }

    std::string readTarget;
    if (!read(element, "default", readTarget)) {
        log_error() << "No default target specified or default target is not a string.";
        return ReturnCode::FAILURE;
    }

    if (parseTarget(readTarget)) {
        e.m_defaultTarget = readTarget;
    } else {
        log_error() << "Default target '" << readTarget << "' is not a supported target.Invalid target.";
        return ReturnCode::FAILURE;
    }

    m_entries.push_back(e);

    // --- TEMPORARY WORKAROUND ---
    // in the wait of activate() being rewritten
    if ("ACCEPT" == e.m_defaultTarget) {
        m_internetAccess = true;
        m_gateway = "10.0.3.1";
    }
    // ----------------------------
    return ReturnCode::SUCCESS;
}

ReturnCode NetworkGateway::parseRule(const json_t *element, std::vector<IPTableEntry::Rule> &rules)
{
    IPTableEntry::Rule r;
    std::string target;
    if (!read(element, "target", target)) {
        log_error() << "Target not specified in the network config";
        return ReturnCode::FAILURE;
    }

    if (parseTarget(target)) {
        r.target = target;
    } else {
        log_error() << target << " is not a valid target.";
        return ReturnCode::FAILURE;
    }

    if (!read(element, "host", r.host)) {
        log_error() << "Host not specified in the network config.";
        return ReturnCode::FAILURE;
    }

    // Parsing different port formats
    json_t *port = json_object_get(element, "port");
    if (port != nullptr) {
        parsePort(port, r.ports);
    }
    // If there were no port configured, leave the port list empty
    // and assume that all ports should be considered in the rule.

    rules.push_back(r);
    return ReturnCode::SUCCESS;
}

ReturnCode NetworkGateway::parsePort(const json_t *element,  IPTableEntry::portFilter &ports)
{
    // Port formatted as single integer means there is only one port
    if (json_is_integer(element)) {
        auto port = json_integer_value(element);
        ports.any = true;
        ports.multiport = false;
        ports.ports = std::to_string(port);
    // Port formatted as a string representing a range
    } else if (json_is_string(element)) {
        auto portRange = json_string_value(element);
        ports.any = true;
        ports.multiport = true;
        ports.ports = portRange;
    // Port formatted as a list of integers
    } else if (json_is_array(element)) {
        size_t ix;
        json_t *val;
        std::string portList = "";

        json_array_foreach(element, ix, val) {
            if (!json_is_integer(val)) {
                log_error() << "Entry in port array is not an integer.";
                return ReturnCode::FAILURE;
            }

            int port = json_integer_value(val);
            portList = portList + std::to_string(port) + ",";
        }
        portList.pop_back();
        ports.any = true;
        ports.multiport = true;
        ports.ports = portList;
    } else {
        log_error() << "Rules specified in an invalid format";
        return ReturnCode::FAILURE;
    }
    return ReturnCode::SUCCESS;
}

bool NetworkGateway::parseTarget(const std::string &str)
{
    if (str == "ACCEPT" || str == "DROP" || str == "REJECT") {
        return true;
    }
    return false;
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
        for (auto entry:m_entries) {
            entry.applyRules();
        }
        return up();
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

std::string IPTableEntry::getChain() {
    if ( "INCOMING" == m_type) {
        return "INPUT";
    } else if ("OUTGOING" == m_type) {
        return "OUTPUT";
    } else if ("FORWARD" == m_type) {
        return "FORWARD";
    }
    return "";
}

bool IPTableEntry::applyRules()
{
    for (auto rule:m_rules) {
        if (!insertRule(rule, getChain())) {
            log_error() << "Couldn't apply the rule " << rule.target;
            return false;
        }
    }

    return true;
}

bool IPTableEntry::insertRule(Rule rule, std::string type)
{
    if (type.empty()) {
        log_error() << "Chain type is not recognized :" << type;
        return false;
    }

    std::string iptableCommand = "iptables -A " + type;

    if (!rule.host.empty()) {
        iptableCommand = iptableCommand + " -s " + rule.host ;
    }

    if (rule.ports.any) {
        if (rule.ports.multiport) {
            iptableCommand = iptableCommand + " -p tcp --match multiport --sports " + rule.ports.ports;
        } else {
            iptableCommand = iptableCommand + " -p tcp --sport " + rule.ports.ports;
        }

    }

    iptableCommand = iptableCommand + " -j " + rule.target;
    log_debug() << "Add network rule : " <<  iptableCommand;

    system(iptableCommand.c_str());
    return true;
}
