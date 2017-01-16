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

#pragma once

#include <jansson.h>

#include "softwarecontainer-common.h"

namespace softwarecontainer {

class CGroupsParser
{
    LOG_DECLARE_CLASS_CONTEXT("CGPA", "CGroups Gateway Parser");

public:
    CGroupsParser();
    /*
     * @brief Parse given configuration element to the settings
     *
     * @param element : represents configuration element
     */
    void parseCGroupsGatewayConfiguration(const json_t *element);

    /*
     * @brief Get the map of cgroup settings
     *
     * @return A list of cgroup settings ready to be applied
     */
    const std::map<std::string, std::string> &getSettings();
private :
    std::map<std::string, std::string> m_settings;

    /*
     * @brief Regulate the setting value due to its multiplier
     *
     * @param settingValue : decimal value without unit information
     * @param multiply     : multiplication value either KB, MB or GB
     *
     * @return A string to regulated final value
     */
    std::string convertToBytes(const std::string settingValue, const int multiply);

    /*
     * @brief Converts the setting value to bytes
     *
     * @param settingValue : value to be applied to a setting
     *
     * @return A string to a value in bytes
     */
    std::string suffixCorrection(const std::string settingValue);
};

} // namespace softwarecontainer
