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

#include "devicenodelogic.h"
#include "devicenodegateway.h"

namespace softwarecontainer {

std::shared_ptr<Device> DeviceNodeLogic::findDeviceByName(const std::string name)
{
    std::vector<std::shared_ptr<Device>>::iterator item =
            std::find_if(m_devList.begin(), m_devList.end(),
            [&] (std::shared_ptr<Device> const &d) { return d->getName() == name; });

    return (item == std::end(m_devList)) ? nullptr : *item;
}

bool DeviceNodeLogic::updateDeviceList(Device &dev)
{
    auto item = findDeviceByName(dev.getName());

    if (item == nullptr) {
        std::shared_ptr<Device> newDevice(new Device(dev));
        m_devList.push_back(std::move(newDevice));
    } else {
        item->calculateDeviceMode(dev.getMode());
    }

    return true;
}

const std::vector<std::shared_ptr<Device>> &DeviceNodeLogic::getDevList()
{
    return m_devList;
}




} // namespace softwarecontainer
