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

#include "softwarecontainererror.h"


namespace softwarecontainer {

class ConfigStoreError : public SoftwareContainerError
{
public:
    ConfigStoreError():
        m_message("Config Store error")
    {
    }

    ConfigStoreError(const std::string &message):
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
 * @brief An error occured in ConfigStore relating to the path to the Service Manifest(s)
 *
 * This exception should be used when the path to the file or directory
 * pointing to one or more Service Manifests is incorrect.
 *
 */
class ServiceManifestPathError : public ConfigStoreError
{
public:
    ServiceManifestPathError():
        ConfigStoreError("Service Manifest path error")
    {
    }

    ServiceManifestPathError(const std::string &message):
        ConfigStoreError(message)
    {
    }

    virtual const char *what() const throw()
    {
        return m_message.c_str();
    }
};

/**
 * @brief An error occured in ConfigStore when parsing a Service Manifest
 *
 * This exception should be used when the Service Manifest is incorrectly
 * formatted, e.g. not a json file.
 *
 */
class ServiceManifestParseError : public ConfigStoreError
{
public:
    ServiceManifestParseError():
        ConfigStoreError("Config Store error - Service Manifest parse error")
    {
    }

    ServiceManifestParseError(const std::string &message):
        ConfigStoreError(message)
    {
    }

    virtual const char *what() const throw()
    {
        return m_message.c_str();
    }
};

/**
 * @brief An error occured in ConfigStore when parsing a Capability in a Service Manifest
 *
 * This exception should be used when a Capability is incorrectly formatted.
 *
 */
class CapabilityParseError : public ConfigStoreError
{
public:
    CapabilityParseError():
        ConfigStoreError("Config Store error - Capability parse error")
    {
    }

    CapabilityParseError(const std::string &message):
        ConfigStoreError(message)
    {
    }

    virtual const char *what() const throw()
    {
        return m_message.c_str();
    }
};

} // namespace softwarecontainer
