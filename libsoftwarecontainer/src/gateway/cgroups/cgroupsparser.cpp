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
#include "cgroupsgateway.h"
#include "jsonparser.h"

#include <math.h>

namespace softwarecontainer {

CGroupsParser::CGroupsParser()
    : m_settings()
{
}

std::string CGroupsParser::convertToBytes(const std::string settingValue, const int multiply)
{
    char *end;
    long long limit_in_bytes = std::strtoll(settingValue.c_str(), &end, 10);
    if (limit_in_bytes ==  std::numeric_limits<long long>::max() ||
        limit_in_bytes ==  std::numeric_limits<long long>::min()) {
        if (errno == ERANGE) {
            std::string errMessage = settingValue + " is out of range";
            log_error() << errMessage;
            throw LimitRangeError(errMessage);
        }
    }

    if (limit_in_bytes > llrint(std::numeric_limits<long long>::max() / multiply)) {
        std::string errMessage = settingValue + " * " + std::to_string(multiply) + " is out of range";
        log_error() << errMessage;
        throw LimitRangeError(errMessage);
    }

    limit_in_bytes *= multiply;
    return std::to_string(limit_in_bytes);
}

std::string CGroupsParser::suffixCorrection(const std::string settingValue)
{
    auto suffix = settingValue.back();
    std::string removeUnit = settingValue;
    removeUnit.pop_back();

    if (std::isdigit(suffix)) {
        //the value is digit already no need to do anything
        return settingValue;
    } else if ('K' == suffix || 'k' == suffix) {
        //the value is Kilobyte unit convert it to byte
        int KB = 1024;
        return convertToBytes(removeUnit, KB);
    } else if ('M' == suffix || 'm' == suffix) {
        //the value is Megabyte unit convert it to byte
        int MB = 1024 * 1024;
        return convertToBytes(removeUnit, MB);
    } else if ('G' == suffix || 'g' == suffix) {
        //the value is Gigabyte unit convert it to byte
        int GB = 1024 * 1024 * 1024;
        return convertToBytes(removeUnit, GB);
    }
    std::string errMessage = "Bad suffix on setting value " + settingValue;
    log_error() << errMessage;
    throw BadSuffixError(errMessage);
}

void CGroupsParser::parseCGroupsGatewayConfiguration(const json_t *element)
{
    std::string settingKey;
    std::string settingValue;

    if (!JSONParser::read(element, "setting", settingKey)) {
        std::string errMessage = "Key \"setting\" either not a string or not in json configuration";
        log_error() << errMessage;
        throw JSonError(errMessage);
    }

    if (!JSONParser::read(element, "value", settingValue)) {
        std::string errMessage = "Key \"value\" either not a string or not in json configuration";
        log_error() << errMessage;
        throw JSonError(errMessage);
    }

    if (("memory.limit_in_bytes" == settingKey) || ("memory.memsw.limit_in_bytes" == settingKey)) {
        settingValue = suffixCorrection(settingValue);

        if (m_settings.count(settingKey)) {
            // if the new value is greater than old one set is as the value
            if (std::stoll(m_settings[settingKey]) < std::stoll(settingValue)) {
                m_settings[settingKey] = settingValue;
            }
        } else {
            m_settings[settingKey] = settingValue;
        }
    } else {
        log_warning() << settingKey << " is not supported by CGroups Gateway" ;
        m_settings[settingKey] = settingValue;
    }
}

const std::map<std::string, std::string> &CGroupsParser::getSettings()
{
    return m_settings;
}
} // namespace softwarecontainer
