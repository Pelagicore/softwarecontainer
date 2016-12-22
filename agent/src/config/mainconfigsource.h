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

#include "softwarecontainer-common.h"
#include "configloader.h"
#include "configtypes.h"
#include "configsource.h"
#include "configdefinition.h"


namespace softwarecontainer {

class MainConfigSource : public ConfigSource
{

LOG_DECLARE_CLASS_CONTEXT("CFGM", "Main config source");

public:
    /**
     * @throws ConfigUnknownError If any config item found in the loaded config
     *                            does not match what is defined in ConfigDefinition
     *
     * @throws ConfigFileError If a known config item could not be parsed
     */
    MainConfigSource(std::unique_ptr<ConfigLoader> loader, TypeMap typeMapping);

    ~MainConfigSource() {}

    std::vector<StringConfig> stringConfigs() override;
    std::vector<IntConfig> intConfigs() override;
    std::vector<BoolConfig> boolConfigs() override;

private:
    /*
     * Can throw ConfigFileError if the underlying Glib load operation fails
     */
    void loadConfig();

    /*
     * Can throw ConfigFileError is the underlying Glib read operation fails
     */
    void readLoadedConfig();

    void createConfigItems();

    /*
     * Can throw ConfigUnknownError if any config item found in the loaded config
     * does not match what is defined in ConfigDefinition
     */
    ConfigType typeOf(const std::string &group, const std::string &key);

    void create(const std::string &group, const std::string &key, ConfigType type);

    template<typename T, typename U>
    T createConfig(const std::string &group, const std::string &name);

    /*
     * Can throw ConfigFileError if a config item can not be parsed
     */
    template<typename T>
    T getGlibValue(const std::string &group, const std::string &key) const;

    std::unique_ptr<ConfigLoader> m_loader;
    TypeMap m_typeMapping;
    std::unique_ptr<Glib::KeyFile> m_config;
    std::map<std::string, std::vector<std::string>> m_configsByGroup;
    std::vector<StringConfig> m_stringConfigs;
    std::vector<IntConfig> m_intConfigs;
    std::vector<BoolConfig> m_boolConfigs;
};

} // namespace softwarecontainer
