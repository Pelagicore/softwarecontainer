
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
#include "configsource.h"
#include "configdefinition.h"

namespace softwarecontainer {


/**
 * @class Config
 *
 * @brief Represents the configuration of SoftwareContainer (the component)
 *
 * Config gathers all config items present from config sources, and provides the
 * values for these configs when asked. Config handles logic for what is to be considered
 * mandatory or optional, and dependencies between configs.
 *
 * Config reads all config items from the sources on creation, and stores them for
 * retrieval when users of Config asks for a config value.
 *
 * Config item uniqueness is based on the combination of config group and config key.
 * Therefore, it is OK to have the same key in multiple groups, but a key must be unique
 * within a group.
 *
 * Some configs are optional and some are mandatory. A config can become mandatory if
 * some other specified config depends on it. E.g. if an optional config in the main config
 * file is uncommented and thus used, it might mean that other configs are mandatory
 * as a consequence.
 *
 * It is an error if a mandatory config can not be found or if one or more dependencies
 * to a config can not be found.
 *
 * There are three different config sources:
 *   * Command line options
 *   * Main config file
 *   * Defaults
 *
 * The config sources as considered in the above order, i.e. first command line options, then
 * the main config file etc. If a config was retrieved from more than one source, i.e. duplicates,
 * only the value of the highest prioritized source will be returned, and the other(s) ignored.
 *
 * It is an error if a config requested by the user of Config is not found, even if the config
 * is optional.
 */
class Config
{

LOG_DECLARE_CLASS_CONTEXT("CFG", "Config");

public:
    /**
     * @brief Constructor - retrieves all configs from the available sources
     *
     * This constructor retrieves all ConfigItem objects from the available config sources,
     * and keeps them for later access by users of Config.
     *
     * @param sources A list of config sources implementing the ConfigSource interface
     * @param mandatory Configs that are to be considered mandatory
     * @param dependencies Dependee configs and their respective dependencies
     *
     * @throws ConfigMandatoryError If any mandatory config is missing, i.e. it is not
     *                              returned by any config source.
     *
     * @throws ConfigDependencyError If one of more dependencies are not met, i.e. there
     *                               is a config present that has dependencies and all of
     *                               those dependencies could not be found.
     */
    Config(std::vector<std::unique_ptr<ConfigSource>> sources,
           MandatoryConfigs mandatory,
           ConfigDependencies dependencies);

    ~Config() {}

    /**
     * @brief getStringValue Get a config value of type string
     *
     * @param group The name of the config group the value belongs to
     * @param key The key for the config value
     *
     * @return A string with the config value
     *
     * @throws ConfigNotFoundError If the config identified with 'group' and 'key' couldn't
     *                             be found
     */
    std::string getStringValue(const std::string &group, const std::string &key) const;

    /**
     * @brief getIntValue Get a config value of type int
     *
     * @param group The name of the config group the value belongs to
     * @param key The key for the config value
     *
     * @return An int with the config value
     *
     * @throws ConfigNotFoundError If the config identified with 'group' and 'key' couldn't
     *                             be found
     */
    int getIntValue(const std::string &group, const std::string &key) const;

    /**
     * @brief getBoolValue Get a config value of type bool
     *
     * @param group The name of the config group the value belongs to
     * @param key The key for the config value
     *
     * @return A bool with the config value
     *
     * @throws ConfigNotFoundError If the config identified with 'group' and 'key' couldn't
     *                             be found
     */
    bool getBoolValue(const std::string &group, const std::string &key) const;

private:
    /*
     * Goes through all dependency relations and makes sure all dependencies for any
     * found dependee have been found in any of the sources.
     */
    void assertDependencies();

    /*
     * Goes through all mandatory configs and make sure they have been found in any
     * of the sources.
     */
    void assertAllMandatoryPresent();

    /*
     * Get all config items from all sources. A call to this method populates
     * m_stringConfigs, m_intConfigs, m_boolConfigs, and m_allConfigs
     */
    void readConfigsFromSources();

    /*
     * Returns a config item from 'configs' that matches 'group' and 'key'
     */
    template<typename T>
    T getConfig(const std::string &group,
                const std::string &key,
                const std::vector<T> &configs) const;

    /*
     * Returns the config item from 'configs' that has the highest prioritized source
     */
    template<typename T>
    T prioritizedConfig(const std::vector<T> &configs) const;

    /*
     * Returns true is all config items in 'dependencies' have been found in the sources
     */
    bool allDepsSatisfied(const std::vector<UniqueKey> &dependencies) const;

    std::vector<std::unique_ptr<ConfigSource>> m_sources;
    MandatoryConfigs m_mandatory;
    ConfigDependencies m_dependencies;
    std::vector<StringConfig> m_stringConfigs;
    std::vector<IntConfig> m_intConfigs;
    std::vector<BoolConfig> m_boolConfigs;
    std::vector<UniqueKey> m_allConfigs;
};

} // namespace softwarecontainer
