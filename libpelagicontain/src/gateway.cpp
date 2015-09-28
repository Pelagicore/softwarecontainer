/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gateway.h"

constexpr const char *Gateway::XDG_RUNTIME_DIR_VARIABLE_NAME;

bool Gateway::setConfig(const std::string &config)
{
    JSonElement rootElement(config);

    bool success = true;

    std::vector<JSonElement> elements;
    rootElement.readChildren(elements);
    for (auto &element : elements) {
        if (isError(readConfigElement(element))) {
            log_warning() << "Could not read config element";
            success = false;
        }
    }

    return success;
}
