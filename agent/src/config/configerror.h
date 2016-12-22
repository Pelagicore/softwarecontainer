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

#include <exception>


namespace softwarecontainer {

class ConfigError : public std::exception
{
public:
    ConfigError():
        m_message("SoftwareContainer configuration error")
    {
    }

    ConfigError(const std::string &message):
        m_message(message)
    {
    }

    virtual const char *what() const throw()
    {
        return m_message.c_str();
    }

protected:
    std::string m_message;
};


/**
 * @brief An error occured which is an internal error in the config code
 *
 * This exception should be used when execution ends up in an internally
 * erroneous code path, i.e. this should not happen.
 */
class ConfigInternalError : public ConfigError
{
public:
    ConfigInternalError():
        ConfigError("Configuration error - fatal internal error in config code")
    {
    }

    ConfigInternalError(const std::string &message):
        ConfigError(message)
    {
    }
};


class ConfigDependencyError : public ConfigError
{
public:
    ConfigDependencyError():
        ConfigError("Configuration error - missing config dependency")
    {
    }

    ConfigDependencyError(const std::string &message):
        ConfigError(message)
    {
    }
};


class ConfigMandatoryError : public ConfigError
{
public:
    ConfigMandatoryError():
        ConfigError("Configuration error - missing mandatory config")
    {
    }

    ConfigMandatoryError(const std::string &message):
        ConfigError(message)
    {
    }
};


/**
 * @brief A config is found which is not defined in ConfigDefinition
 */
class ConfigUnknownError : public ConfigError
{
public:
    ConfigUnknownError():
        ConfigError("Configuration error - unknown config")
    {
    }

    ConfigUnknownError(const std::string &message):
        ConfigError(message)
    {
    }
};


/**
 * @brief An error occured in a Glib operation
 */
class ConfigFileError : public ConfigError
{
public:
    ConfigFileError():
        ConfigError("Configuration error - config file error")
    {
    }

    ConfigFileError(const std::string &message):
        ConfigError(message)
    {
    }
};


/**
 * @brief A requested config item could not be found in any of the config sources
 */
class ConfigNotFoundError : public ConfigError
{
public:
    ConfigNotFoundError():
        ConfigError("Configuration error - wrong group and/or key")
    {
    }

    ConfigNotFoundError(const std::string &message):
        ConfigError(message)
    {
    }
};

} // namespace softwarecontainer
