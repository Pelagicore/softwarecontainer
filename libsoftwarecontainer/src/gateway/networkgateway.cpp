
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

ReturnCode NetworkGateway::readConfigElement(const json_t *element)
{
    Entry e;
    if (!read(element, "type", e.type)) {
        log_error() << " \"internet-access\" ";
        return ReturnCode::FAILURE;
    }

    const json_t *rules = json_object_get(element, "rules");

    size_t ix;
    json_t *val;
    json_array_foreach(rules, &ix, val) {
        if (json_is_object(arrayElement)) {
            if (isError(parseRule(arrayElement, e.rules))) {
                log_error() << "Could not read rule config";
                return false;
            }
        } else {
            log_error() << "formatting of rules array is incorrect.";
            return false;
        }
    }

    std::string readTarget;
    if (!read(element, "default", readTarget) {
        log_error() << "No default target specified or default target is not a string.";
        return ReturnCode::FAILURE;
    }

    e.defaultTarget = parseTarget(readTarget);
    if (e.defaultTarget == Target::INVALID_TARGET) {
        log_error() << "Default target '" << readTarget << "' is not a supported target.Invalid target.";
        return ReturnCode::FAILURE;
    }

    m_entries.push_back(e);
    return ReturnCode::SUCCESS;
}

ReturnCode NetworkGateway::parseRule(const json_t *element, std::vector<Rule> &rules)
{
    Rule r;
    std::string target;
    if (!read(element, "target", target) {
        log_error() << "Target not specified in the network config";
        return ReturnCde::FAILURE;
    }

    r.target = parseTarget(target);
    if (r.target == Target::INVALID_TARGET) {
        log_error() << target << " is not a valid target.";
        return ReturnCode::FAILURE;
    }

    if (!read(element, "host", r.host)) {
        log_error() << "Host not specified in the network config.";
        return ReturnCode::FAILURE;
    }

    // Parsing different port formats
    // Port formatted as single integer
    json_t *port = json_object_get(element, "port");
    if (json_is_integer(port)) {
        int iport = json_integer_value(port);
        r.ports.push_back(iport);

    // Port formatted as a string representing a range
    json_t *port = json_object_get(element, "port");
    } else if (json_is_string(port)) {
        std::string portRange = json_string_value(port);
        
        const std::string::size_type n = portRange.find("-");
        const std::string first = portRange.substr(0, n);
        const std::string last = portRange.substr(n + 1);

        int startPort;
        if (!parseInt(first.c_str(), &startPoint)) {
             log_error() << "Starting port in range " << portRange << "is not an integer.";
             return ReturnCode::FAILURE;
        }

        int endPort;
        if (!parseInt(first.c_str(), &endPoint)) {
             log_error() << "End port in range " << portRange << "is not an integer.";
             return ReturnCode::FAILURE;
        }

        for (int i = startPort; i <= endPort; ++i) {
            r.ports.push_back(i);
        }

    // Port formatted as a list of integers
    } else if (json_is_array(port)) {
        size_t ix;
        json_t *val;
        json_array_foreach(port, ix, val) {
            if (!json_is_integerval)) {
                log_error() << "Entry in port array is not an integer.";
                return ReturnCode::FAILURE;
            }

            int iport = json_integer_value(port);
            r.ports.push_back(iport);
        } 
    }

    rules.push_back(r);
    return ReturnCode::SUCCESS;
}

Target NetworkGateway::parseTarget(const std::string &str)
{
    if (str == "ACCEPT") {
        return Target::ACCEPT;
    }
    
    if (str == "DROP") {
        return Target::DROP;
    }

    if (str == "REJECT") {
        return Target::REJECT;
    }
    return Target::INVALID_TARGET;
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

    if (!isBridgeAvailable()) {
        log_error() << "Bridge not available.";
        return false;
    }

    if (m_internetAccess) {
        generateIP();
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

    m_ip = m_generator.gen_ip_addr(ipAddrNet);
    log_debug() << "IP set to " << m_ip;

    return true;
}

bool NetworkGateway::setDefaultGateway()
{
    log_debug() << "Attempting to set default gateway";
    std::string cmdDel = "route del default gw " + m_gateway;
    executeInContainer(cmdDel);

    std::string cmdAdd = "route add default gw " + m_gateway;
    if (isError(executeInContainer(cmdAdd))) {
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
        cmd = "ifconfig eth0 " + ip() + " netmask 255.255.255.0 up";
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
