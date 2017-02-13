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

#include "iptableentry.h"
#include "gateway/gateway.h"
#include <iostream>

namespace softwarecontainer {

bool IPTableEntry::applyRules()
{
    for (auto rule : m_rules) {
        if (rule.protocols.size()) {

            for (auto proto : rule.protocols) {
                if (!insertCommand(interpretRuleWithProtocol(rule, proto))) {
                    std::cerr << "Couldn't apply the rule " << rule.target << std::endl;
                    return false;
                }
            }

        } else if (!insertCommand(interpretRule(rule))) {
            std::cerr << "Couldn't apply the rule " << rule.target << std::endl;
            return false;
        }
    }

    if (!insertCommand(interpretPolicy())) {
        std::cerr << "Unable to set policy " << convertTarget(m_defaultTarget)
                  << " for " << m_type << std::endl;
        return false;
    }

    return true;
}

std::string IPTableEntry::convertTarget (Target& t)
{
    switch(t)
    {
        case Target::ACCEPT:
            return "ACCEPT";
        case Target::DROP:
            return "DROP";
        case Target::REJECT:
            return "REJECT";
        case Target::INVALID_TARGET:
            return "INVALID";
    }
    return "INVALID";
}

std::string IPTableEntry::interpretRuleWithProtocol(Rule rule, const std::string &protocol)
{
    std::string iptableCommand = "iptables -A " + m_type;

    if (!rule.host.empty()) {
        if (rule.host != "*") {
            if ("INPUT" == m_type) {
                iptableCommand = iptableCommand + " -s ";
            } else {
                iptableCommand = iptableCommand + " -d ";
            }
            iptableCommand = iptableCommand + rule.host ;
        }
    }

    if (rule.ports.any) {
        if (rule.ports.multiport) {
            iptableCommand = iptableCommand + " -p " + protocol + " --match multiport ";

            if ("INPUT" == m_type) {
                iptableCommand = iptableCommand + "--sports " + rule.ports.ports;
            } else {
                iptableCommand = iptableCommand + "--dports " + rule.ports.ports;
            }
        } else {
            iptableCommand = iptableCommand + " -p " + protocol + " ";
            if ("INPUT" == m_type) {
                iptableCommand = iptableCommand + "--sport " + rule.ports.ports;
            } else {
                iptableCommand = iptableCommand + "--dport " + rule.ports.ports;
            }
        }
    } else {
        iptableCommand = iptableCommand + " -p " + protocol;
    }

    iptableCommand = iptableCommand + " -j " + convertTarget(rule.target);

    return iptableCommand;
}

std::string IPTableEntry::interpretRule(Rule rule)
{
    std::string iptableCommand = "iptables -A " + m_type;

    if (!rule.host.empty()) {
        if (rule.host != "*") {
            if ("INPUT" == m_type) {
                iptableCommand = iptableCommand + " -s ";
            } else {
                iptableCommand = iptableCommand + " -d ";
            }
            iptableCommand = iptableCommand + rule.host ;
        }
    }

    if (rule.ports.any) {
        if (rule.ports.multiport) {
            iptableCommand = iptableCommand + " -p all --match multiport ";

            if ("INPUT" == m_type) {
                iptableCommand = iptableCommand + "--sports " + rule.ports.ports;
            } else {
                iptableCommand = iptableCommand + "--dports " + rule.ports.ports;
            }
        } else {
            iptableCommand = iptableCommand + " -p tcp ";
            if ("INPUT" == m_type) {
                iptableCommand = iptableCommand + "--sport " + rule.ports.ports;
            } else {
                iptableCommand = iptableCommand + "--dport " + rule.ports.ports;
            }
        }
    }

    iptableCommand = iptableCommand + " -j " + convertTarget(rule.target);

    return iptableCommand;
}

std::string IPTableEntry::interpretPolicy()
{
    if (m_defaultTarget != Target::ACCEPT && m_defaultTarget != Target::DROP) {
        std::cerr << "Wrong default target : " << convertTarget(m_defaultTarget) << std::endl;
        return "";
    }
    std::string iptableCommand = "iptables -P " + m_type + " " + convertTarget(m_defaultTarget);

    return iptableCommand;
}


bool IPTableEntry::insertCommand(std::string command)
{
    std::cout << "Add network rule : " <<  command << std::endl;

    try {
        Glib::spawn_command_line_sync(command);

    } catch (Glib::SpawnError e) {
        std::cerr << "Failed to spawn " << command << ": code " << e.code()
                  << " msg: " << e.what() << std::endl;

        return false;
    } catch (Glib::ShellError e) {
        std::cerr << "Failed to call " << command << ": code " << e.code()
                  << " msg: " << e.what() << std::endl;
        return false;
    }

    return true;
}

std::string IPTableEntry::toString()
{
    return "IPTableEntry type: " + m_type;
}

} // namespace softwarecontainer
