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

#include <sys/stat.h>
#include <sys/types.h>

#include "gateway/devicenode/devicenodegateway.h"
#include "functionjob.h"

namespace softwarecontainer {

DeviceNodeGateway::DeviceNodeGateway() :
    Gateway(ID)
{
}

ReturnCode DeviceNodeGateway::readConfigElement(const json_t *element)
{
    DeviceNodeParser parser;
    DeviceNodeParser::Device dev;

    if (isError(parser.parseDeviceNodeGatewayConfiguration(element, dev))) {
        log_error() << "Could not parse device node configuration";
        return ReturnCode::FAILURE;
    }

    return m_logic.updateDeviceList(dev);
}

bool DeviceNodeGateway::activateGateway()
{
    auto returnValue = applySettings();
    return isSuccess(returnValue);
}

bool DeviceNodeGateway::teardownGateway()
{
    return true;
}

ReturnCode DeviceNodeGateway::applySettings()
{
    auto devlist = m_logic.getDevList();
    if (devlist.empty()) {
        log_info() << "Activate was called when no devices has been configured.";
        return ReturnCode::FAILURE;
    }

    for (auto &dev : devlist) {
        log_info() << "Mapping device " << dev.name;

        if (dev.major != -1) {

            // mknod dev.name c dev.major dev.minor
            FunctionJob job(getContainer(), [&] () {
                auto err =  mknod(dev.name.c_str(), S_IFCHR | dev.mode,
                             makedev(dev.major, dev.minor));
                if (err) {
                    log_error() << "Err happened while trying to run mknod in container" <<
                                   " error: " << err << " - " << strerror(errno);
                }
                return err;
            });

            job.start();

            if (job.wait() != 0) {
                log_error() << "Failed to create device " << dev.name;
                return ReturnCode::FAILURE;
            }
        } else {
            // No major & minor numbers specified => simply map the device from
            // the host into the container
            getContainer()->mountDevice(dev.name);

            if (dev.mode != -1) {
                FunctionJob job(getContainer(), [&] () {
                    return chmod(dev.name.c_str(), dev.mode);
                });
                job.start();

                if (job.wait() != 0) {
                    log_error() << "Could not 'chmod " << dev.mode
                                << "' the mounted device " << dev.name;
                    return ReturnCode::FAILURE;
                }
            }
        }
    }
    return ReturnCode::SUCCESS;
}

} // namespace softwarecontainer
