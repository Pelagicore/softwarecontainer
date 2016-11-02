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

#pragma once

#ifndef IPTABLEENTRY_H_
#define IPTABLEENTRY_H_

#include "softwarecontainer-common.h"

/**
 * @brief A rules entry for the treatment of packets.
 */
class IPTableEntry
{
    LOG_DECLARE_CLASS_CONTEXT("IPTE", "IPTable Entry");
public:
    IPTableEntry() : m_priority{0}, m_type{""}, m_defaultTarget{INVALID_TARGET} {};
    /**
     * @brief container for port filtering options. Used internally in a Rule.
     */
    struct portFilter {
        portFilter() : any{0}, multiport{0}, ports{""} {};
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
        portFilter  ports;
        Target target;
    };

    /**
     * @brief Applies all rules to iptables
     * @return true  Upon success
     * @return false Upon failure
     */
    ReturnCode applyRules();

    unsigned int m_priority;
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
    ReturnCode insertRule(Rule rule);
};



#endif /* IPTABLEENTRY_H_ */
