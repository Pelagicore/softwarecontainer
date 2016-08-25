
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

CgroupsGateway::CgroupsGateway()
    : Gateway(ID)
    , m_settings({})
{
    m_state = GatewayState::CREATED;
}

ReturnCode CgroupsGateway::readConfigElement(const JSonElement &element)
{
    if (!element.isValid()) {
        log_error() << "Error: invalid JSON data";
        return ReturnCode::FAILURE;
    }

    if (!element.isObject()) {
        log_error() << "Error: Elements must be JSON objects";
        return ReturnCode::FAILURE;
    }

    const json_t *data = element.root();
    json_t *setting;
    json_t *value;

    setting = json_object_get(data, "setting");
    if (!json_is_string(setting)) {
        log_error() << "Error: setting is not a string";
        return ReturnCode::FAILURE;
    }
    std::string settingString = json_string_value(setting);

    value = json_object_get(data, "value");
    if (!json_is_string(value)) {
        log_error() << "Error, value is not a string";
        return ReturnCode::FAILURE;
    }

    std::string valueString = json_string_value(value);
    if (m_settings.count(settingString) > 0) {
        log_warning() << "setting '" << settingString << "' is set more than once, may be problematic.";
    }
    m_settings.insert( std::pair<std::string, std::string>(settingString, valueString) );

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

    return success == ReturnCode::SUCCESS;
}

bool CgroupsGateway::teardownGateway()
{
    return true;
}
