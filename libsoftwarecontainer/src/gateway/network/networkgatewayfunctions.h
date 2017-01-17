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

#include <netinet/in.h>

#include "softwarecontainer-common.h"

namespace softwarecontainer {
/**
 * @brief Sets up and manages network access and routing to the container
 *
 * The responsibility of NetworkGateway is to setup network connection as specified by given
 * configuration. This configuration is described in detail in the user documentation, but
 * a short summary of it is how to handle incoming and outgoing network packages using the
 * three targets: ACCEPT, DROP and REJECT.
 */
class NetworkGatewayFunctions
{
    LOG_DECLARE_CLASS_CONTEXT("NGF", "Network Gateway Functions");
public:
    /**
     * @brief Generate IP address for the container
     *
     * Generates an IP address and returns it
     *
     * Note that a file on the system acts as a placeholder for the DHCP server.
     * The file keeps track of the highest used IP address.
     *
     * @param netmask    : representation available bits
     * @param gatewayIP  : representation of the gateway IP
     * @param cnrainerID : representation of the containerID
     *
     * @return uint32_t indicates assigned IP address
     */
    uint32_t generateIP(const uint32_t netmask,
                        const std::string gatewayIP,
                        const int32_t containerID);
};

} // namespace softwarecontainer
