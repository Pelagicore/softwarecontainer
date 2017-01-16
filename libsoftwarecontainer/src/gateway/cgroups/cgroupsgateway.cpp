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

#include "jansson.h"

#include "softwarecontainer-common.h"
#include "cgroupsgateway.h"

namespace softwarecontainer {

CgroupsGateway::CgroupsGateway()
    : Gateway(ID)
    , m_parser()
{
}

ReturnCode CgroupsGateway::readConfigElement(const json_t *element)
{
    try {
        m_parser.parseCGroupsGatewayConfiguration(element);
    } catch (CgroupsGatewayError &e) {
        log_error() << "Could not parse CGroups configuration element";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

bool CgroupsGateway::activateGateway()
{
    ReturnCode success = ReturnCode::FAILURE;
    auto cgroupSettings = m_parser.getSettings();
    for (auto& setting: cgroupSettings) {
        success = getContainer()->setCgroupItem(setting.first, setting.second);
        if (success != ReturnCode::SUCCESS) {
            log_error() << "Error activating Cgroups Gateway, could not set cgroup item "
                        << setting.first << ": " << setting.second;
            break;
        }
    }

    return isSuccess(success);
}

bool CgroupsGateway::teardownGateway()
{
    return true;
}

} // namespace softwarecontainer
