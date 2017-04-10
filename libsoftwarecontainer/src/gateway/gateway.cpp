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

#include "gateway.h"

namespace softwarecontainer {


Gateway::Gateway(const std::string &id,
                 std::shared_ptr<ContainerAbstractInterface> container,
                 bool isDynamic) :
    m_activatedOnce(false),
    m_id(id),
    m_container(container),
    m_isDynamic(isDynamic),
    m_state(GatewayState::CREATED)
{
}

std::string Gateway::id() const
{
    return m_id;
}

bool Gateway::setConfig(const json_t *config)
{
    if (GatewayState::ACTIVATED == m_state && !m_isDynamic) {
        std::string message = "Can not configure a gateway that is already activated "
                              "if the gateway does not support dynamic behavior. "
                              "Gateway ID: " + id();
        log_error() << message;
        throw GatewayError(message);
    }

    if (!json_is_array(config)) {
        log_error() << "Root JSON element is not an array";
        return false;
    }

    if (json_array_size(config) == 0) {
        log_error() << "Root JSON array is empty";
        return false;
    }

    for(size_t i = 0; i < json_array_size(config); i++) {
        json_t *element = json_array_get(config, i);
        if (!json_is_object(element)) {
            log_error() << "json configuration is not an object";
            return false;
        }

        if (!readConfigElement(element)) {
            log_warning() << "Could not read config element";
            return false;
        }
    }

    m_state = GatewayState::CONFIGURED;
    return true;
}

bool Gateway::activate() {
    if (GatewayState::ACTIVATED == m_state && !m_isDynamic) {
        std::string message =  "Can not activate a gateway that is already activated "
                               "if the gateway does not support dynamic behavior. "
                               "Gateway ID: " + id();
        log_error() << message;
        throw GatewayError(message);
    }

    if (GatewayState::CONFIGURED != m_state) {
        std::string message = "Activate was called on a gateway which is not in configured state. "
                              "Gateway ID: " + id();
        log_error() << message;
        throw GatewayError(message);
    }

    if (!activateGateway()) {
        log_error() << "Couldn't activate gateway: " << id();
        return false;
    }

    m_state = GatewayState::ACTIVATED;
    return true;
}

bool Gateway::teardown() {
    /* At this point, a gateway should either be in state ACTIVATED if it is non-dynamic, or
       if it is dynamic it should have been activated at least once before.
     */
    if (GatewayState::ACTIVATED != m_state && !m_activatedOnce) {
        std::string message = "Teardown called on non-activated gateway. Gateway ID:  " + id();
        log_error() << message;
        throw GatewayError(message);
    }

    if (!teardownGateway()) {
        log_error() << "Could not tear down gateway: " << id();
        return false;
    }

    // Return to a state of nothingness
    m_state = GatewayState::CREATED;

    /* Since we have been torn down, we should not be considered to have been
       activated any more. */
    m_activatedOnce = false;

    return true;
}

std::shared_ptr<ContainerAbstractInterface> Gateway::getContainer()
{
    std::shared_ptr<ContainerAbstractInterface> ptrCopy = m_container;
    return ptrCopy;
}

bool Gateway::isConfigured()
{
    return m_state >= GatewayState::CONFIGURED;
}

bool Gateway::isActivated()
{
    // For dynamic gateways it's only relevant to know if it has been activated
    // at least once, the current state is not important
    if (m_isDynamic) {
        return m_activatedOnce;
    }

    // For non-dynamic gateways, the current state is the only relevant info
    return m_state >= GatewayState::ACTIVATED;
}

} // namespace softwarecontainer
