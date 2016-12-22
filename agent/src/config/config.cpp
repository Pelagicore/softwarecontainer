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

#include "softwarecontainer-common.h"
#include "config.h"
#include "configtypes.h"
#include "configerror.h"


namespace softwarecontainer {


Config::Config(std::vector<std::unique_ptr<ConfigSource>> sources,
               MandatoryConfigs mandatory,
               ConfigDependencies dependencies):
    m_sources(std::move(sources)),
    m_mandatory(mandatory),
    m_dependencies(dependencies),
    m_stringConfigs(std::vector<StringConfig>()),
    m_intConfigs(std::vector<IntConfig>()),
    m_boolConfigs(std::vector<BoolConfig>()),
    m_allConfigs(std::vector<UniqueKey>())
{
    /*
     * The order of the calls below is important since some methods modifies members that are
     * needed in subsequent method calls.
     */

    // Read all ConfigItems from all sources and store them
    readConfigsFromSources();

    // If any ConfigItem has one or more dependencies which are not present, it is a fatal error
    assertDependencies();

    // If any group-key combo is mandatory and not present, it is a fatal error
    assertAllMandatoryPresent();
}

void Config::assertDependencies()
{
    for (const auto &dep : m_dependencies) {
        UniqueKey config = dep.first;
        std::string group = std::get<0>(config);
        std::string key = std::get<1>(config);

        // Check if the dependee exists in the configs we got from the sources
        bool found = std::find(m_allConfigs.begin(), m_allConfigs.end(), config) != m_allConfigs.end();

        // If a dependee was present among the configs, all its dependencies needs to be present as well
        if (found && !allDepsSatisfied(dep.second)) {
            // It is a fatal error if one or more dependencies were not found
            std::string message = "One or more dependencies were not met for config " + group + "::" + key;
            log_error() << message;
            throw ConfigDependencyError(message);
        }
    }
}

bool Config::allDepsSatisfied(const std::vector<UniqueKey> &dependencies) const
{
    for (const UniqueKey &dependency : dependencies) {
        bool found = std::find(m_allConfigs.begin(), m_allConfigs.end(), dependency) != m_allConfigs.end();

        if (!found) {
            return false;
        }
    }

    return true;
}

void Config::assertAllMandatoryPresent()
{
    for (const UniqueKey &mandatory : m_mandatory) {
        // A mandatory config must be found among the configs we got from the sources
        bool found = std::find(m_allConfigs.begin(), m_allConfigs.end(), mandatory) != m_allConfigs.end();

        if (!found) {
            std::string message = "Mandatory config " + mandatory.first + "::" + mandatory.second + " not found";
            log_error() << message;
            throw ConfigMandatoryError(message);
        }
    }
}

void Config::readConfigsFromSources()
{
    for (auto &source : m_sources) {
        auto stringConfigs = source->stringConfigs();
        m_stringConfigs.insert(m_stringConfigs.end(), stringConfigs.begin(), stringConfigs.end());

        auto intConfigs = source->intConfigs();
        m_intConfigs.insert(m_intConfigs.end(), intConfigs.begin(), intConfigs.end());

        auto boolConfigs = source->boolConfigs();
        m_boolConfigs.insert(m_boolConfigs.end(), boolConfigs.begin(), boolConfigs.end());
    }

    // Put all configs in the same place for later use 
    for (auto &config : m_stringConfigs) {
        m_allConfigs.push_back(UniqueKey(config.group(), config.key()));
    }

    for (auto &config : m_intConfigs) {
        m_allConfigs.push_back(UniqueKey(config.group(), config.key()));
    }

    for (auto &config : m_boolConfigs) {
        m_allConfigs.push_back(UniqueKey(config.group(), config.key()));
    }
}

std::string Config::getStringValue(const std::string &group, const std::string &key) const
{
    return getConfig(group, key, m_stringConfigs).value();
}

int Config::getIntValue(const std::string &group, const std::string &key) const
{
    return getConfig(group, key, m_intConfigs).value();
}

bool Config::getBoolValue(const std::string &group, const std::string &key) const
{
    return getConfig(group, key, m_boolConfigs).value();
}

template<typename T>
T Config::getConfig(const std::string &group, const std::string &key, const std::vector<T> &configs) const
{
    std::vector<T> foundConfigs;

    for (auto config : configs) {
        if (config.group() == group && config.key() == key) {
            log_debug() << "Found config "
                        << group << "::" << key
                        << " in "
                        << ConfigTypes::configSourceToString(config.source());
            foundConfigs.push_back(config);
        }
    }

    if (foundConfigs.empty()) {
        std::string message = "No config found for " + group + "::" + key;
        log_error() << message;
        throw ConfigNotFoundError(message);
    }

    T config = prioritizedConfig(foundConfigs);

    log_debug() << "Using config "
                << group << "::" << key
                << " with value \""
                << config.value() << "\""
                << ", from " << ConfigTypes::configSourceToString(config.source());

    return config;
}

template<typename T>
T Config::prioritizedConfig(const std::vector<T> &configs) const
{
    for (auto config : configs) {
        if (config.source() == ConfigSourceType::Commandline) {
            return config;
        }
    }

    for (auto config : configs) {
        if (config.source() == ConfigSourceType::Main) {
            return config;
        }
    }

    for (auto config : configs) {
        if (config.source() == ConfigSourceType::Default) {
            return config;
        }
    }

    log_error() << "Config has an incorrect source";
    throw ConfigInternalError("Config has an incorrect source");
}

} // namespace softwarecontainer
