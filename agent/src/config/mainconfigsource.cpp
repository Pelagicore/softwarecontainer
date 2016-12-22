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

#include "mainconfigsource.h"
#include "config.h"
#include "configerror.h"


namespace softwarecontainer {

MainConfigSource::MainConfigSource(std::unique_ptr<ConfigLoader> loader, TypeMap typeMapping):
    m_loader(std::move(loader)),
    m_typeMapping(std::move(typeMapping)),
    m_configsByGroup(std::map<std::string, std::vector<std::string>>()),
    m_stringConfigs(std::vector<StringConfig>()),
    m_intConfigs(std::vector<IntConfig>()),
    m_boolConfigs(std::vector<BoolConfig>())
{
    log_debug() << "Initializing main config source";

    /*
     * The order of the calls below is important as some calls modify members
     * that subsequent calls uses. In general, all errors that can occur are turned
     * into ConfigError of various kinds and thrown where the error happened.
     * These exceptions are not caught and handled anywhere, they are propagated to
     * the creator of MainConfigSource.
     */

    loadConfig();
    readLoadedConfig();
    createConfigItems();
}

std::vector<StringConfig> MainConfigSource::stringConfigs()
{
    return m_stringConfigs;
}

std::vector<IntConfig> MainConfigSource::intConfigs()
{
    return m_intConfigs;
}

std::vector<BoolConfig> MainConfigSource::boolConfigs()
{
    return m_boolConfigs;
}

void MainConfigSource::loadConfig()
{
    try {
        m_config = m_loader->loadConfig();
    } catch (Glib::Error &error) {
        throw ConfigFileError("Error loading main config file");
    }
}

void MainConfigSource::readLoadedConfig()
{
    try {
        std::vector<Glib::ustring> groups = m_config->get_groups();
        for (auto &group : groups) {
            std::vector<Glib::ustring> keys = m_config->get_keys(group);

            for (auto &key : keys) {
                m_configsByGroup[group].push_back(key);
            }
        }
    } catch (Glib::Error &error) {
        throw ConfigFileError("Error reading main config file");
    }
}

void MainConfigSource::createConfigItems()
{
    for (auto &configGroup : m_configsByGroup) {
        auto group = configGroup.first;
        auto keys = configGroup.second;

        for (auto &key : keys) {
            ConfigType type = typeOf(group, key);
            create(group, key, type);
        }
    }
}

ConfigType MainConfigSource::typeOf(const std::string &group, const std::string &key)
{
    try {
        return m_typeMapping.at(std::make_pair(group, key));
    } catch (std::exception &error) {
        std::string message = "Unkown config " + group + "::" + key;
        log_error() << message;
        throw ConfigUnknownError(message);
    }
}

void MainConfigSource::create(const std::string &group, const std::string &key, ConfigType type)
{
    switch (type) {
        case ConfigType::String:
            m_stringConfigs.push_back(createConfig<StringConfig, std::string>(group, key));
            break;
        case ConfigType::Integer:
            m_intConfigs.push_back(createConfig<IntConfig, int>(group, key));
            break;
        case ConfigType::Boolean:
            m_boolConfigs.push_back(createConfig<BoolConfig, bool>(group, key));
            break;
        default:
            break;
    }
}

template<typename T, typename U>
T MainConfigSource::createConfig(const std::string &group, const std::string &name)
{
    U value = getGlibValue<U>(group, name);
    T config(group, name, value);
    config.setSource(ConfigSourceType::Main);

    return config;
}

template<>
std::string MainConfigSource::getGlibValue(const std::string &group, const std::string &key) const
{
    try {
        std::string value = m_config->get_string(Glib::ustring(group), Glib::ustring(key));
        return value;
    } catch (Glib::KeyFileError &error) {
        std::string message = "Could not parse string value: \"" + error.what() + "\"";
        log_error() << message;
        throw ConfigFileError(message);
    }

    throw ConfigInternalError();
}

template<>
int MainConfigSource::getGlibValue(const std::string &group, const std::string &key) const
{
    try {
        int value = m_config->get_integer(Glib::ustring(group), Glib::ustring(key));
        return value;
    } catch (Glib::KeyFileError &error) {
        std::string message = "Could not parse integer value: \"" + error.what() + "\"";
        log_error() << message;
        throw ConfigFileError(message);
    }

    throw ConfigInternalError();
}

template<>
bool MainConfigSource::getGlibValue(const std::string &group, const std::string &key) const
{
    try {
        bool value = m_config->get_boolean(Glib::ustring(group), Glib::ustring(key));
        return value;
    } catch (Glib::KeyFileError &error) {
        std::string message = "Could not parse boolean value: \"" + error.what() + "\"";
        log_error() << message;
        throw ConfigFileError(message);
    }

    throw ConfigInternalError();
}

} // namespace softwarecontainer
