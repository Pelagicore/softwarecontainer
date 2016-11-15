/*
 * Copyright (C) 2016 Pelagicore AB
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


bool Gateway::setConfig(const std::string &config)
{
    if (m_state == GatewayState::ACTIVATED) {
        log_error() << "Can't configure a gateway that is already activated: " << id();
    }

    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);
    if (!root) {
        log_error() << "Could not parse config: " << error.text;
        return false;
    }

    if (!json_is_array(root)) {
        log_error() << "Root JSON element is not an array";
        return false;
    }

    if (json_array_size(root) == 0) {
        log_error() << "Root JSON array is empty";
        return false;
    }

    for(size_t i = 0; i < json_array_size(root); i++) {
        json_t *element = json_array_get(root, i);
        if (!json_is_object(element)) {
            log_error() << "json configuration is not an object";
            return false;
        }

        if (isError(readConfigElement(element))) {
            log_error() << "Could not read config element";
            return false;
        }
    }

    m_state = GatewayState::CONFIGURED;
    return true;
}

bool Gateway::activate() {
    if (m_state == GatewayState::ACTIVATED) {
        log_warning() << "Activate was called on a gateway which was already activated: " << id();
        return false;
    }

    if (m_state != GatewayState::CONFIGURED) {
        log_warning() << "Activate was called on a gateway which is not in configured state: " << id();
        return false;
    }

    if (!hasContainer()) {
        log_warning() << "Activate was called on a gateway which has no associated container: " << id();
        return false;
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
        return false;
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

ReturnCode Gateway::setEnvironmentVariable(const std::string &variable, const std::string &value)
{
    if (hasContainer()) {
        return getContainer()->setEnvironmentVariable(variable, value);
    } else {
        log_error() << "Can't set environment variable on gateway without container";
        return ReturnCode::FAILURE;
    }
}

/**
 * @brief Execute the given command in the container
 */
ReturnCode Gateway::executeInContainer(const std::string &cmd)
{
    if (hasContainer()) {
        return getContainer()->executeInContainer(cmd);
    } else {
        log_error() << "Can't execute in container from gateway without container";
        return ReturnCode::FAILURE;
    }
}

ReturnCode Gateway::executeInContainer(ContainerFunction func)
{
    if (hasContainer()) {
        pid_t pid;
        ReturnCode ret = getContainer()->executeInContainer(func, &pid);
        if (isSuccess(ret)) {
            waitpid(pid, 0, 0);
        }

        return ret;
    } else {
        log_error() << "Can't execute in container from gateway without container";
        return ReturnCode::FAILURE;
    }
}
