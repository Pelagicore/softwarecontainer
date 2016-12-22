
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
#include "configitem.h"


namespace softwarecontainer {

/**
 * @class ConfigItem
 * @brief Base class for all config item types
 */
ConfigItem::ConfigItem(const std::string &group, const std::string &name):
    m_group(group),
    m_name(name),
    m_missing(false),
    m_mandatory(false)
{
}

std::string ConfigItem::key() const
{
    return m_name;
}

std::string ConfigItem::group() const
{
    return m_group;
}

void ConfigItem::setSource(ConfigSourceType type)
{
    m_source = type;
}

ConfigSourceType ConfigItem::source() const
{
    return m_source;
}


/**
 * @class StringConfig
 * @brief Represents a config item of type string
 */
StringConfig::StringConfig(const std::string &group, const std::string &name, std::string value):
    ConfigItem(group, name),
    m_value(value)
{
}

std::string StringConfig::value()
{
    return m_value;
}


/**
 * @class IntConfig
 * @brief Represents a config item of type int
 */
IntConfig::IntConfig(const std::string &group, const std::string &name, int value):
    ConfigItem(group, name),
    m_value(value)
{
}

int IntConfig::value()
{
    return m_value;
}


/**
 * @class BoolConfig
 * @brief Represents a config item of type bool
 */
BoolConfig::BoolConfig(const std::string &group, const std::string &name, bool value):
    ConfigItem(group, name),
    m_value(value)
{
}

bool BoolConfig::value()
{
    return m_value;
}

} // namespace softwarecontainer
