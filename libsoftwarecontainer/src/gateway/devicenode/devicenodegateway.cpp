
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

#include "devicenodegateway.h"
#include "devicenodelogic.h"

#include <sys/stat.h>
#include <sys/types.h>


DeviceNodeGateway::DeviceNodeGateway() :
    Gateway(ID)
{
}

ReturnCode DeviceNodeGateway::readConfigElement(const json_t *element)
{
    DeviceNodeParser parser;
    DeviceNodeLogic logic;
    DeviceNodeParser::Device dev;

    if (isError(parser.parseDeviceNodeGatewayConfiguration(element, dev))) {
        log_error() << "Could not parse device node configuration";
        return ReturnCode::FAILURE;
    }

    auto item = std::find_if(m_devList.begin(), m_devList.end(),
        [&] (DeviceNodeParser::Device const &d) { return d.name == dev.name; });

    if (item == std::end(m_devList)) {
        m_devList.push_back(dev);
    } else {
        if ((item->major == dev.major) && (item->major == dev.minor)) {
            item->mode = logic.calculateDeviceMode(item->mode, dev.mode);
        } else {
            //now we have a new device with same name
            m_devList.push_back(dev);
        }
    }

    m_devList.push_back(dev);
    return ReturnCode::SUCCESS;
}


bool DeviceNodeGateway::activateGateway()
{
    if (m_devList.empty()) {
        log_info() << "Activate was called when no devices has been configured.";
        return false;
    }

    for (auto &dev : m_devList) {
        log_info() << "Mapping device " << dev.name;

        if (dev.major != -1) {

            // mknod dev.name c dev.major dev.minor
            pid_t pid = INVALID_PID;
            getContainer()->executeInContainer([&] () {
                return mknod(dev.name.c_str(), S_IFCHR | dev.mode,
                             makedev(dev.major, dev.minor));
            }, &pid);

            if (waitForProcessTermination(pid) != 0) {
                log_error() << "Failed to create device " << dev.name;
                return false;
            }
        } else {
            // No major & minor numbers specified => simply map the device from
            // the host into the container
            getContainer()->mountDevice(dev.name);

            if (dev.mode != -1) {
                pid_t pid = INVALID_PID;
                getContainer()->executeInContainer([&] () {
                    return chmod(dev.name.c_str(), dev.mode);
                }, &pid);

                if (waitForProcessTermination(pid) != 0) {
                    log_error() << "Could not 'chmod " << dev.mode
                                << "' the mounted device " << dev.name;
                    return false;
                }
            }
        }
    }

    return true;
}

bool DeviceNodeGateway::teardownGateway()
{
    return true;
}
