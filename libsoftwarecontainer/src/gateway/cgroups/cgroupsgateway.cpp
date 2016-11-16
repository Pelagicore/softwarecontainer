
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


#include "jansson.h"

#include "softwarecontainer-common.h"
#include "cgroupsgateway.h"
#include "cgroupsparser.h"

CgroupsGateway::CgroupsGateway()
    : Gateway(ID)
    , m_settings({})
{
    m_state = GatewayState::CREATED;
}

ReturnCode CgroupsGateway::readConfigElement(const json_t *element)
{
    CGroupsParser::CGroupsPair pair;
    CGroupsParser parser;

    if (isError(parser.parseCGroupsGatewayConfiguration(element, pair))) {
        log_error() << "Could not parse CGroups configuration element";
        return ReturnCode::FAILURE;
    }

    m_settings.insert(pair);
    return ReturnCode::SUCCESS;
}

bool CgroupsGateway::activateGateway()
{
    ReturnCode success = ReturnCode::FAILURE;
    for (auto& setting: m_settings) {
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
