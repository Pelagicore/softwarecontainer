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

#include "iptableentry.h"

ReturnCode IPTableEntry::applyRules()
{
    for (auto rule : m_rules) {
        if (ReturnCode::FAILURE == insertRule(rule)) {
            log_error() << "Couldn't apply the rule " << rule.target;
            return ReturnCode::FAILURE;
        }
    }

    return ReturnCode::SUCCESS;
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

ReturnCode IPTableEntry::insertRule(Rule rule)
{
    std::string iptableCommand = "iptables -A " + m_type;

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

    iptableCommand = iptableCommand + " -j " + convertTarget(rule.target);
    log_debug() << "Add network rule : " <<  iptableCommand;

    try {
        Glib::spawn_command_line_sync(iptableCommand);

    } catch (Glib::SpawnError e) {
        log_error() << "Failed to spawn " << iptableCommand << ": code " << e.code()
                               << " msg: " << e.what();

        return ReturnCode::FAILURE;
    } catch (Glib::ShellError e) {
        log_error() << "Failed to call " << iptableCommand << ": code " << e.code()
                                       << " msg: " << e.what();
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}
