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


namespace softwarecontainer {

/**
 * @brief Represents the type of a config value
 *
 * This is used to specify the type of the a value when mandatory configs are
 * defined. This information is later used to know how a key should be used to
 * retrive a value, e.g. what type specific methods can be used.
 */
enum class ConfigType
{
    String,
    Integer,
    Boolean
};


enum class ConfigSourceType
{
    Commandline,
    Main,
    Default
};


class ConfigTypes
{
public:
    static std::string configSourceToString(const ConfigSourceType &type)
    {
        switch (type) {
            case ConfigSourceType::Commandline:
                return "Commandline options";
            case ConfigSourceType::Main:
                return "Main config file";
            case ConfigSourceType::Default:
                return "Default values";
            default:
                break;
        }

        return std::string();
    }
};

} // namespace softwarecontainer
