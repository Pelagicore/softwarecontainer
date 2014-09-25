/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "generators.h"
#include "jansson.h"

#include "devicenodegateway.h"

DeviceNodeGateway::DeviceNodeGateway():
    Gateway()
{
}

std::string DeviceNodeGateway::id()
{
    return "devicenode";
}

bool DeviceNodeGateway::setConfig(const std::string &config)
{
    json_error_t error;
    json_t       *root = NULL, *devices = NULL;
    bool success = true;
    std::vector<struct Device> newDevList;

    log_debug("DeviceNodeGateway::setConfig called");

    /* Get root JSON object */
    root = json_loads(config.c_str(), 0, &error);

    if (!root) {
        log_error("Error on line %d: %s", error.line, error.text);
        success = false;
        goto cleanup_setConfig;
    }

    // Get string
    devices = json_object_get(root, "devices");

    if (!json_is_array(devices)) {
        log_error("Value is not an array.");
        log_error("error: on line %d: %s", error.line, error.text);
        success = false;
        goto cleanup_setConfig;
    }

    newDevList = parseDeviceList(devices, success);
    if (success) {
        m_devList = newDevList;
    }

cleanup_setConfig:

    // Also frees 'devices'
    if (root)
        json_decref(root);

    return success;
}

std::vector<DeviceNodeGateway::Device>
DeviceNodeGateway::parseDeviceList(json_t *list, bool &ok) {
    ok = true;
    std::vector<struct DeviceNodeGateway::Device> dev_list;
    for (size_t i = 0; i < json_array_size(list); i++) {
        struct DeviceNodeGateway::Device dev;
        std::string* fields[] = {&dev.name, &dev.major, &dev.minor, &dev.mode};
        const char* fieldsStr[] = {"name", "major", "minor", "mode"};
        uint numFields = 4;

        json_t *device = json_array_get(list, i);
        if (ok && !json_is_object(device)) {
            log_error("Expected JSON device object, found something else");
            ok = false;
        }

        for (size_t j = 0; ok && j < numFields; j++) {
            json_t *value = 0;
            value = json_object_get(device, fieldsStr[j]);
            if (ok && !json_is_string(value)) {
                log_error() << "Key '" << fieldsStr[j] << "' is not a member of device object!";
                ok = false;
            }
            if (json_string_value(value)) {
                *fields[j] = std::string(json_string_value(value));
                ok = true;
            } else {
                ok = false;
                break;
            }
        }

        if (ok) {
            dev_list.push_back(dev);
        } else {
            break;
        }
    }
    return dev_list;
}

bool DeviceNodeGateway::activate()
{
    for(auto& dev : m_devList)
    {
        auto success = systemCall("mknod " + dev.name + " c " +
                                                dev.major + " " + dev.minor);
        if (!isError(success)) {
            success = systemCall("chmod " +
                                                    dev.mode + " " + dev.name );
        } else {
            log_error() << "Failed to create device " << dev.name;
            return false;
        }
    }

    return true;
}
