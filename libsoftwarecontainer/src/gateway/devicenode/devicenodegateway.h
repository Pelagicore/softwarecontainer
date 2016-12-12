
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

#include "jansson.h"

#include "softwarecontainer-common.h"
#include "gateway.h"
#include "devicenodelogic.h"

/**
 * @brief This gateway is responsible for exposing device nodes in an LXC container.
 * The basic operation looks as follows:
 * - DeviceNodeGateway (DNG) is loaded with a JSON configuration detailing a
 *   list of devices to create, with device names (mandatory), permission modes (optional),
 *   major  (optional) and minor numbers  (optional).
 *
 * Notes:
 * - activate() will stop creating devices in the container upon first failure
*
 */
class DeviceNodeGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("DNG", "Device node gateway");

public:
    static constexpr const char *ID = "devicenode";

    DeviceNodeGateway();

    ~DeviceNodeGateway()
    {
    }

    virtual ReturnCode readConfigElement(const json_t *element) override;

    /**
     * @brief Implements Gateway::activateGateway
     *
     * This function will iterate over all devices and issue mknod and chmod commands, which are run in the
     * container.
     * @return true upon success; all commands executed successfully, false
     *              otherwise
     */
    virtual bool activateGateway() override;

    /**
     * @brief Implements Gateway::teardownGateway
     */
    virtual bool teardownGateway() override;

private:
    /*
     * @brief apply all setting of device node list
     *
     * @return success or failure due to state of operation
     */
    ReturnCode applySettings(void);

    DeviceNodeLogic m_logic;
};
