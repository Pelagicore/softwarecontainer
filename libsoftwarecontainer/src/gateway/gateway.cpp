/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gateway.h"

bool Gateway::setConfig(const std::string &config)
{
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);
    if (!root) {
        log_error() << "Could not parse config: " << error.text;
        return false;
    }

    if (json_is_array(root)) {
        for(size_t i = 0; i < json_array_size(root); i++) {
            json_t *element = json_array_get(root, i);
            if (isError(readConfigElement(element))) {
                log_error() << "Could not read config element";
                return false;
            }
        }
    } else {
        log_error() << "Root JSON element is not an array";
        return false;
    }

    m_state = GatewayState::CONFIGURED;
    return true;
}

bool Gateway::activate() {
    if (m_state != GatewayState::CONFIGURED) {
        log_warning() << "Activate was called on a gateway which is not in configured state: " << id();
        return false;
    }

    if (!hasContainer()) {
        log_warning() << "Activate was called on a gateway which has no associated container: " << id();
        return false;
    }

    return activateGateway();
}

bool Gateway::teardown() {
    if (m_state != GatewayState::ACTIVATED) {
        log_warning() << "Teardown called on non-activated gateway " << id();
    }

    return teardownGateway();
}
