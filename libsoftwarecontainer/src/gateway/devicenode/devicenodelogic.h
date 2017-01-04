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
#include "devicenodeparser.h"

namespace softwarecontainer {

/**
 * @brief This class is responsible for storing all device node configurations
 * in a well-formed list.
 */
class DeviceNodeLogic
{
    LOG_DECLARE_CLASS_CONTEXT("DNL", "Device node logic");

public:
    DeviceNodeLogic() {};

    /*
     * @brief Calculate the device node mode after applying white-listing policy
     * i.e. if the appliedMode is 706 and storedMode is 622 then
     *      the calculated mode will be 726
     *
     * @return integer value representing new device mode
     */
    int calculateDeviceMode(int storedMode, int appliedMode);

    /*
     * @brief Find device node which has the same name with the argument
     *
     * @return Iterator to the founded device node element or end of the list
     * if no such element is found.
     */
    std::vector<DeviceNodeParser::Device>::iterator findDeviceByName(const std::string name);

    /*
     * @brief Update the device node list due to white-listing policy
     * i.e. if the device name does not exist new device node will be added
     *      if the device name exist and major and minor minor device IDs are also matched
     *      then update the  mode of device node
     *
     * @return either SUCCESS or FAILURE due to state of operation
     */
    ReturnCode updateDeviceList(DeviceNodeParser::Device dev);

    /*
     * @brief Get the list of device nodes
     *
     * @return A list of well-formed device nodes ready to be applied
     */
    const std::vector<DeviceNodeParser::Device> &getDevList();

private:
    std::vector<DeviceNodeParser::Device> m_devList;
};

} // namespace softwarecontainer
