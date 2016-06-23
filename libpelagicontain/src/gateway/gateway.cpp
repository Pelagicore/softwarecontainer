/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gateway.h"
#include <fcntl.h>

// TODO: Move this to somewhere reasonable
constexpr const char *Gateway::XDG_RUNTIME_DIR_VARIABLE_NAME;

bool Gateway::setConfig(const std::string &config)
{
    bool success = true;
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
                log_warning() << "Could not read config element";
                success = false;
            }
        }
    }
    return success;
}
