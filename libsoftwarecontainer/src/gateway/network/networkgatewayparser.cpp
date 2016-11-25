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
#include "networkgatewayparser.h"

ReturnCode NetworkGatewayParser::parseNetworkGatewayConfiguration(const json_t *element, IPTableEntry &e)
{
    std::string chain;
    if (!JSONParser::read(element, "direction", chain)) {
        log_error() << "No type specified in network config.";
        return ReturnCode::FAILURE;
    }

    e.m_defaultTarget = IPTableEntry::Target::DROP;

    if ("INCOMING" == chain) {
        e.m_type = "INPUT";
    } else if ("OUTGOING" == chain) {
        e.m_type = "OUTPUT";
    } else {
        log_error() << e.m_type << " is not a valid type ('INCOMING' or 'OUTGOING')";
        return ReturnCode::FAILURE;
    }

    const json_t *rules = json_object_get(element, "allow");

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

    return ReturnCode::SUCCESS;
}


ReturnCode NetworkGatewayParser::parseRule(const json_t *element, std::vector<IPTableEntry::Rule> &rules)
{
    IPTableEntry::Rule r;

    r.target = IPTableEntry::Target::ACCEPT;

    if (!JSONParser::read(element, "host", r.host)) {
        log_error() << "Host not specified in the network config.";
        return ReturnCode::FAILURE;
    }

    // Parsing port formats
    json_t *ports = json_object_get(element, "ports");
    if (ports != nullptr) {
        parsePort(ports, r.ports);
    }

    // Parsing protocols
    json_t *protocols = json_object_get(element, "protocols");
    if (protocols != nullptr) {
        parseProtocol(protocols, r.protocols);
    }

    rules.push_back(r);
    return ReturnCode::SUCCESS;
}

ReturnCode NetworkGatewayParser::parsePort(const json_t *element, IPTableEntry::portFilter &ports)
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

bool NetworkGatewayParser::isProtocolValid(std::string protocol) {
    if ((protocol ==  "tcp") ||  (protocol ==  "udp") || (protocol == "icmp")) {
        return true;
    } else {
        log_error() << protocol
                    << " is not valid value. Only tcp, udp and icmp protocols are allowed";
        return false;
    }
}

ReturnCode NetworkGatewayParser::parseProtocol(const json_t *element,
                                               std::vector<std::string> &proto)
{
    // Single Protocol
    if (json_is_string(element)) {
        std::string protocol = json_string_value(element);
        if (!isProtocolValid(protocol)) {
            return ReturnCode::FAILURE;
        }
        proto.push_back(protocol);
    // Multiple protocols
    } else if (json_is_array(element)) {
        size_t ix;
        json_t *val;

        json_array_foreach(element, ix, val) {
            if (!json_is_string(val)) {
                log_error() << "Listed protocol is not valid";
                return ReturnCode::FAILURE;
            }

            std::string protocol = json_string_value(val);

            if (!isProtocolValid(protocol)) {
                return ReturnCode::FAILURE;
            }

            proto.push_back(protocol);
        }
    } else {
        log_error() << "Protocols specified in an invalid format";
        return ReturnCode::FAILURE;
    }
    return ReturnCode::SUCCESS;
}
