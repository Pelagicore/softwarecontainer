
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

#pragma once

#include "softwarecontainer-common.h"


namespace softwarecontainer {

// Forward declared to avoid circular include with config.h
enum class ConfigType;


class MandatoryConfigs
{

LOG_DECLARE_CLASS_CONTEXT("CFGM", "SoftwareContainer general config mandatory values");

public:
    MandatoryConfigs();

    ~MandatoryConfigs() {}

    /**
     * @brief Adds a mandatory config key with associated group
     *
     * Both the group and key will become mandatory.
     *
     * @param group The name of the group the key is in
     * @param key The name of the config key
     * @param type A ConfigType specifying the type of the config value
     */
    void addConfig(const std::string &group, const std::string &key, ConfigType type);

    /**
     * @brief Get all mandatory configs as group and key
     *
     * @returns A list of group-key-type tuples
     *
     * TODO: Do we really need to have the group present here again?
     */
    std::vector<std::tuple<std::string, std::string, ConfigType>> configs();

    /**
     * @brief Get all mandatory config groups
     *
     * @returns A list of group names
     */
    std::vector<std::string> groups();

private:
    std::vector<std::tuple<std::string, std::string, ConfigType>> m_configs;
    std::vector<std::string> m_groups;
};

} // namespace softwarecontainer
