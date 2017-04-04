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

#include "gateway/gateway.h"
#include "cgroupsparser.h"
#include "softwarecontainererror.h"

namespace softwarecontainer {

/**
 * @brief The cgroups gateway sets cgroups related settings for the container.
 */

class CgroupsGateway: public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("CGRO", "Cgroups gateway");

public:
    static constexpr const char *ID = "cgroups";

    CgroupsGateway(std::shared_ptr<ContainerAbstractInterface> container);
    ~CgroupsGateway() { }

    bool readConfigElement(const json_t *element) override;
    bool activateGateway() override;
    bool teardownGateway() override;

private:
    CGroupsParser m_parser;
};

class CgroupsGatewayError : public SoftwareContainerError
{
public:
    CgroupsGatewayError():
        m_message("Cgroups Gateway exception")
    {
    }

    CgroupsGatewayError(const std::string &message):
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

class LimitRangeError : public CgroupsGatewayError
{
public:
    LimitRangeError():
        CgroupsGatewayError("Range error is occurred when trying to limit memory")
    {
    }

    LimitRangeError(const std::string &message):
        CgroupsGatewayError(message)
    {
    }
};

class BadSuffixError : public CgroupsGatewayError
{
public:
    BadSuffixError():
        CgroupsGatewayError("The configuration being applied has a bad suffix")
    {
    }

    BadSuffixError(const std::string &message):
        CgroupsGatewayError(message)
    {
    }
};

class JSonError : public CgroupsGatewayError
{
public:
    JSonError():
        CgroupsGatewayError("A configuration error has occured")
    {
    }

    JSonError(const std::string &message):
        CgroupsGatewayError(message)
    {
    }
};

class InvalidInputError : public CgroupsGatewayError
{
public:
    InvalidInputError():
        CgroupsGatewayError("The given value is invalid")
    {
    }
    InvalidInputError(const std::string &message):
        CgroupsGatewayError(message)
    {
    }
};

} // namespace softwarecontainer
