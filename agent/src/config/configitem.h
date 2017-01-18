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
#include "configtypes.h"


namespace softwarecontainer {

class ConfigItem
{
public:
    ConfigItem(const std::string &group, const std::string &name);
    ~ConfigItem() {}

    std::string key() const;
    std::string group() const;
    void setSource(ConfigSourceType type);
    ConfigSourceType source() const;

private:
    std::string m_group;
    std::string m_name;
    ConfigSourceType m_source;
};


class StringConfig : public ConfigItem
{
public:
    StringConfig(const std::string &group, const std::string &name, std::string value);
    ~StringConfig() {}

    std::string value();

private:
    std::string m_value;
};


class IntConfig : public ConfigItem
{
public:
    IntConfig(const std::string &group, const std::string &name, int value);
    ~IntConfig() {}

    int value();

private:
    int m_value;
};


class BoolConfig : public ConfigItem
{
public:
    BoolConfig(const std::string &group, const std::string &name, bool value);
    ~BoolConfig() {}

    bool value();

private:
    bool m_value;
};

} // namespace softwarecontainer
