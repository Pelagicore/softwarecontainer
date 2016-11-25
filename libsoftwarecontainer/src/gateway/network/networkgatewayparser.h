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

#include "softwarecontainer-common.h"
#include "iptableentry.h"
#include "jsonparser.h"


class NetworkGatewayParser
{
    LOG_DECLARE_CLASS_CONTEXT("NETP", "Network Gateway Parser");
public :
    /**
     * @brief Parses NetworkGateway configuration into IPTableEntry
     *
     * @param element JSON string containing configuration
     * @param e rule representing corresponding network rules
     * @returns ReturnCode::SUCCESS if the rule is successfully parsed
     * @return ReturnCode::FAILURE otherwise.
     */
    ReturnCode parseNetworkGatewayConfiguration(const json_t *element, IPTableEntry &e);

private :
    /**
     * @brief Parses a json element to a Rule
     * @return ReturnCode::SUCCESS if the rule is successfully parsed
     * @return ReturnCode::FAILURE otherwise.
     */
    ReturnCode parseRule(const json_t *element, std::vector<IPTableEntry::Rule> &rules);

    /**
     * @brief Parses ports from a json element
     * @return ReturnCode::SUCCESS if the rule is successfully parsed
     * @return ReturnCode::FAILURE otherwise.
     */
    ReturnCode parsePort(const json_t *element, IPTableEntry::portFilter &ports);

    /**
    * @return true or false whether the given protocol is available in NetworkGateway or not
    */
    bool isProtocolValid(std::string protocol);

    /**
     * @brief Parses protocols from a json element
     * @return ReturnCode::SUCCESS if protocols are successfully parsed
     * @return ReturnCode::FAILURE otherwise.
     */
    ReturnCode parseProtocol(const json_t *element, std::vector<std::string> &proto);

    /**
     * @brief Parses a string to a Target
     * @return Either the valid Target representation of true or false
     */
    IPTableEntry::Target parseTarget(const std::string &str);

};
