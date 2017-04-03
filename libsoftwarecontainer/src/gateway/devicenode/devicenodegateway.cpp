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
    Gateway(ID, container, true /*this GW is dynamic*/)
{
}

bool DeviceNodeGateway::readConfigElement(const json_t *element)
{
    Device dev;

    if (!dev.parse(element)) {
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
        if (!dev->activate(getContainer())) {
            return false;
        }
    }
    return true;
}

bool DeviceNodeGateway::teardownGateway()
{
    return true;
}

} // namespace softwarecontainer
