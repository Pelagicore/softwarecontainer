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

bool Gateway::setConfig(const json_t *config)
{
    if (m_state == GatewayState::ACTIVATED) {
        log_error() << "Can not configure a gateway that is already activated: " << id();
        throw GatewayError("Gateway already activated");
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
            log_error() << "Could not read config element";
            return false;
        }
    }

    m_state = GatewayState::CONFIGURED;
    return true;
}

bool Gateway::activate() {
    if (m_state == GatewayState::ACTIVATED) {
        log_error() << "Activate was called on a gateway which was already activated: " << id();
        throw GatewayError("Gateway already activated");
    }

    if (m_state != GatewayState::CONFIGURED) {
        log_error() << "Activate was called on a gateway which is not in configured state: " << id();
        throw GatewayError("Gateway is not configured");
    }

    if (!hasContainer()) {
        log_error() << "Activate was called on a gateway which has no associated container: " << id();
        throw GatewayError("Gateway does not have a container instance");
    }

    if (!activateGateway()) {
        log_error() << "Couldn't activate gateway: " << id();
        return false;
    }

    m_state = GatewayState::ACTIVATED;
    return true;
}

bool Gateway::teardown() {
    if (m_state != GatewayState::ACTIVATED) {
        log_error() << "Teardown called on non-activated gateway: " << id();
        throw GatewayError("Gateway not previosly activated");
    }

    if (!teardownGateway()) {
        log_error() << "Could not tear down gateway: " << id();
        return false;
    }

    // Return to a state of nothingness
    m_state = GatewayState::CREATED;
    return true;
}

bool Gateway::hasContainer()
{
    return m_container != nullptr;
}

std::shared_ptr<ContainerAbstractInterface> Gateway::getContainer()
{
    if (!hasContainer()) {
        throw GatewayError("Attempting to get container reference before any container has been assigned.");
    }

    std::shared_ptr<ContainerAbstractInterface> ptrCopy = m_container;
    return ptrCopy;
}

void Gateway::setContainer(std::shared_ptr<ContainerAbstractInterface> container)
{
    m_container = container;
}

bool Gateway::isConfigured()
{
    return m_state >= GatewayState::CONFIGURED;
}

bool Gateway::isActivated()
{
    return m_state >= GatewayState::ACTIVATED;
}

} // namespace softwarecontainer
