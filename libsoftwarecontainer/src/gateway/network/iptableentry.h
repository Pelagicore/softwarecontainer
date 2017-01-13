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

#pragma once

#include "softwarecontainer-common.h"

namespace softwarecontainer {

/**
 * @brief A rules entry for the treatment of packets.
 */
class IPTableEntry
{
    LOG_DECLARE_CLASS_CONTEXT("IPTE", "IPTable Entry");
public:
    IPTableEntry() : m_type{""}, m_defaultTarget{DROP} {};
    /**
     * @brief container for port filtering options. Used internally in a Rule.
     */
    struct portFilter {
        portFilter(bool _any=0, bool _multiport=0, std::string _ports="") :
            any(_any),
            multiport(_multiport),
            ports(_ports)
            {};
        bool any;
        bool multiport;
        std::string ports;
    };

    /**
     * @brief Targets for Rules
     */
    enum Target
    {
        INVALID_TARGET,
        ACCEPT,
        DROP,
        REJECT
    };

    /**
     * @brief Definition of a 'Rule' used to handle network traffic. Used internally in an Entry.
     */
    struct Rule
    {
        std::string host;
        std::vector<std::string> protocols;
        portFilter  ports;
        Target target;
    };

    /**
     * @brief Applies all rules to iptables
     * @return true  Upon success
     * @return false Upon failure
     */
    ReturnCode applyRules();

    /**
     * @brief Interprets a rule to iptables applicable string
     * @return string indicating interpreted rule
     */
    std::string interpretRule(Rule rule);

    /**
     * @brief Interprets a rule with protocol information to iptables applicable string
     * @return string indicating interpreted rule
     */
    std::string interpretRuleWithProtocol(Rule rule, const std::string &protocol);

    /**
     * @brief This function Interprets defaultTarget rule to iptables applicable policy string
     *
     * defaultTarget indicates what happens to packets if they don't match to any rules.
     * iptables apply this functionality with setting policy. The role of this function is
     * converting defaultTarget configuration value to iptables policy
     * @return string indicating interpreted policy
     * @return empty string when interfered incompatible m_defaultTarget
     */
    std::string interpretPolicy(void);


    /**
     * @brief Creates a string with information about the entry
     */
    std::string toString();

    std::string m_type;
    std::vector<Rule> m_rules;
    Target m_defaultTarget;
private:
    /**
     * @brief converts target to string
     * @return string representation of target
     */
    std::string convertTarget (Target& t);

    /**
     * @brief Inserts a rule to iptables
     * @return true  Upon success
     * @return false Upon failure
     */
    ReturnCode insertCommand(std::string command);
};

} // namespace softwarecontainer
