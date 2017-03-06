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

namespace softwarecontainer {

std::vector<DeviceNodeParser::Device>::iterator
DeviceNodeLogic::findDeviceByName(const std::string name)
{
    std::vector<DeviceNodeParser::Device>::iterator item =
            std::find_if(m_devList.begin(), m_devList.end(),
            [&] (DeviceNodeParser::Device const &d) { return d.name == name; });

    return item;
}

bool DeviceNodeLogic::updateDeviceList(DeviceNodeParser::Device dev)
{
    auto item = findDeviceByName(dev.name);

    if (item == std::end(m_devList)) {
        m_devList.push_back(dev);
    } else {
        item->mode = calculateDeviceMode(item->mode, dev.mode);
    }

    return true;
}

const std::vector<DeviceNodeParser::Device> &DeviceNodeLogic::getDevList()
{
    return m_devList;
}


int DeviceNodeLogic::calculateDeviceMode(const int storedMode, const int appliedMode)
{
    int mode = storedMode;

    if (storedMode != appliedMode) {
        //apply more permissive option for owner
        ((appliedMode / 100) >= (storedMode / 100)) ?
                mode = (appliedMode / 100) * 100 : mode = (storedMode / 100) * 100;
        //apply more permissive option for group
        (((appliedMode / 10) % 10) >= ((storedMode / 10) % 10)) ?
                mode += (((appliedMode / 10) % 10) * 10) : mode += (((storedMode / 10) % 10) * 10);
        //apply more permissive option for others
        ((appliedMode % 10) >= (storedMode % 10)) ?
                mode += (appliedMode % 10) :  mode += (storedMode % 10);

    }

    return mode;
}

} // namespace softwarecontainer
