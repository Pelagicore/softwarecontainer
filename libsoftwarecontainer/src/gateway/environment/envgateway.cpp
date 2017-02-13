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

#include <string>
#include <unistd.h>

#include "envgateway.h"
#include "envgatewayparser.h"

namespace softwarecontainer {

EnvironmentGateway::EnvironmentGateway(std::shared_ptr<ContainerAbstractInterface> container) :
    Gateway(ID, container)
{
}

EnvironmentGateway::~EnvironmentGateway()
{
}

bool EnvironmentGateway::readConfigElement(const json_t *element)
{
    EnvironmentGatewayParser parser;
    EnvironmentGatewayParser::EnvironmentVariable variable;

    if (!parser.parseEnvironmentGatewayConfigElement(element, variable, m_variables)) {
        log_error() << "Could not parse environment gateway element";
        return false;
    }

    m_variables[variable.first] = variable.second;
    return true;
}

bool EnvironmentGateway::activateGateway()
{
    if (m_variables.empty()) {
        log_error() << "No environment variables set in gateway";
        return false;
    }

    for (auto &variable : m_variables) {
        std::shared_ptr<ContainerAbstractInterface> container = getContainer();
        if (!container->setEnvironmentVariable(variable.first, variable.second)) {
            log_error() << "Could not set environment variable " << variable.first
                        << " for the container";
            return false;
        }
    }

    return true;
}

bool EnvironmentGateway::teardownGateway()
{
    return true;
}

} // namespace softwarecontainer
