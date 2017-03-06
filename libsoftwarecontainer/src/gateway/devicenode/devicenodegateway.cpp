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
#include <sys/stat.h>
#include <libgen.h>

#include "gateway/devicenode/devicenodegateway.h"
#include "functionjob.h"

namespace softwarecontainer {

DeviceNodeGateway::DeviceNodeGateway(std::shared_ptr<ContainerAbstractInterface> container) :
    Gateway(ID, container)
{
}

bool DeviceNodeGateway::readConfigElement(const json_t *element)
{
    DeviceNodeParser parser;
    DeviceNodeParser::Device dev;

    if (!parser.parseDeviceNodeGatewayConfiguration(element, dev)) {
        log_error() << "Could not parse device node configuration";
        return false;
    }

    return m_logic.updateDeviceList(dev);
}

bool DeviceNodeGateway::activateGateway()
{
    auto devlist = m_logic.getDevList();
    if (devlist.empty()) {
        log_info() << "Activate was called when no devices has been configured.";
        return false;
    }

    for (auto &dev : devlist) {
        log_info() << "Mapping device " << dev.name;

        std::string devicePathInContainerOnHost = buildPath(getContainer()->rootFS(), dev.name);
        std::string deviceParent = parentPath(devicePathInContainerOnHost);

        // Already existing files can't be converted into directories
        if (existsInFileSystem(deviceParent) && !isDirectory(deviceParent)) {
            log_error() << "Parent path of " << dev.name << " already exist and is not a directory";
            return false;
        }

        // If the parent directory does not already exist, we create it.
        if (!isDirectory(deviceParent) && !createDirectory(deviceParent)) {
            log_error() << "Could not create parent directory for device " << deviceParent;
            return false;
        }

        // Mount device in container
        getContainer()->mountDevice(dev.name);

        // If mode is specified, try to set mode for the mounted device.
        if (dev.mode != -1) {
            FunctionJob job(getContainer(), [&] () {
                return chmod(dev.name.c_str(), dev.mode);
            });
            job.start();
            job.wait();
            if (job.isError()) {
                log_error() << "Could not 'chmod " << dev.mode
                            << "' the mounted device " << dev.name;
                return false;
            }
        }
    }
    return true;
}

bool DeviceNodeGateway::teardownGateway()
{
    return true;
}

} // namespace softwarecontainer
