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

#include "cgroupsparser.h"
#include "jsonparser.h"

namespace softwarecontainer {

CGroupsParser::CGroupsParser()
    : m_settings()
{
}

ReturnCode CGroupsParser::parseCGroupsGatewayConfiguration(const json_t *element)
{
    std::string settingKey;
    std::string settingValue;

    if (!JSONParser::read(element, "setting", settingKey)) {
        log_error() << "Key \"setting\" either not a string or not in json configuration";
        return ReturnCode::FAILURE;
    }

    if (!JSONParser::read(element, "value", settingValue)) {
        log_error() << "Key \"value\" either not a string or not in json configuration";
        return ReturnCode::FAILURE;
    }

    if ("memory.limit_in_bytes" == settingKey) {
        if (m_settings.count(settingKey)) {
            // if the new value is greater than old one set is as the value
            if (std::stoi(m_settings[settingKey]) < std::stoi(settingValue)) {
                m_settings[settingKey] = settingValue;
            }
        } else {
            m_settings[settingKey] = settingValue;
        }
    } else {
        log_warning() << settingKey << " is not supported by CGroups Gateway" ;
        m_settings[settingKey] = settingValue;
    }

    return ReturnCode::SUCCESS;
}

const std::map<std::string, std::string> &CGroupsParser::getSettings()
{
    return m_settings;
}
} // namespace softwarecontainer
