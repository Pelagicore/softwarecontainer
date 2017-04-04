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
    long long limit_in_bytes = std::strtoll(settingValue.c_str(), NULL, 10);
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

        // if the new value is smaller/equal than the old value, then we don't save the new value.
        if (m_settings.count(settingKey) != 0) {
            if (std::stoll(m_settings[settingKey]) >= std::stoll(settingValue)) {
                return;
            }
        }

    } else if (("cpu.shares" == settingKey)) {
        int newValue;
        try {
            newValue = std::stoi(settingValue);
        } catch (std::out_of_range &err) {
            std::string errorMessage = "The value for cpu.shares is too high";
            log_error() << errorMessage;
            throw InvalidInputError(errorMessage);
        } catch (std::invalid_argument &err) {
            std::string errorMessage = "The value for cpu.shares is not an integer";
            log_error() << errorMessage;
            throw InvalidInputError(errorMessage);
        }

        if (newValue < 2) {
            std::string errorMessage = "Value of cpu.shares must be at least 2";
            log_error() << errorMessage;
            throw InvalidInputError(errorMessage);
        }
        // if the new value is smaller/equal to the old value, then we don't save the new value.
        if (m_settings.count(settingKey) != 0) {
            if (std::stoi(m_settings[settingKey]) >= newValue) {
                return;
            }
        }
        settingValue = std::to_string(newValue);
        log_debug() << "Value for cpu.shares: " << settingValue;

    } else if ("net_cls.classid" == settingKey) {
        // Should be of format 0xAAAABBBB
        if (settingValue.find("0x") != 0 // Has to begin with 0x
            || settingValue.length() > 10 // Can not be longer than 0x + 8
            || settingValue.length() < 7) // Has to be at least 0xAAAAB to cover major/minor handle
        {
            std::string errorMessage = "net_cls.classid should be of form 0xAAAABBBB";
            log_error() << errorMessage;
            throw InvalidInputError(errorMessage);
        }

        char *endPtr = NULL;
        unsigned long int hexedValue = strtoul(settingValue.c_str(), &endPtr, 16);
        if (0 == hexedValue || ULONG_MAX == hexedValue || ((endPtr != NULL) && *endPtr != '\0')) {
            std::string errorMessage = "Could not parse all of net_cls.classid value as hex string";
            log_error() << errorMessage;
            throw InvalidInputError(errorMessage);
        }

    } else {
        log_warning() << settingKey << " is not supported by CGroups Gateway" ;
    }

    // If we got this far we save the key/value pair
    m_settings[settingKey] = settingValue;
}

const std::map<std::string, std::string> &CGroupsParser::getSettings()
{
    return m_settings;
}
} // namespace softwarecontainer
