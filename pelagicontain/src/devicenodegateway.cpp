/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "generators.h"
#include "jansson.h"

#include "devicenodegateway.h"

DeviceNodeGateway::DeviceNodeGateway(ControllerAbstractInterface &controllerInterface):
    Gateway(controllerInterface)
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

std::vector<struct DeviceNodeGateway::Device>
DeviceNodeGateway::parseDeviceList(json_t *list, bool &ok) {
    ok = true;
    std::vector<struct DeviceNodeGateway::Device> dev_list;
    for (uint i = 0; i < json_array_size(list); i++) {
        struct DeviceNodeGateway::Device dev;
        json_t *device = 0;
        std::string* fields[] = {&dev.name, &dev.major, &dev.minor, &dev.mode};
        std::string fieldsStr[] = {"name", "major", "minor", "mode"};
        uint numFields = 4;

        device = json_array_get(list, i);
        if (ok && !json_is_object(device)) {
            log_error("Expected JSON device object, found something else");
            ok = false;
        }

        for (uint j = 0; ok && j < numFields; j++) {
            json_t *value = 0;
            value = json_object_get(device, fieldsStr[j].c_str());
            if (ok && !json_is_string(value)) {
                log_error(std::string("Key '" + fieldsStr[j] +
                                      "' is not a member of device object!").c_str());
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
    bool success = true;
    for (uint i = 0; i < m_devList.size(); i++)
    {
        struct DeviceNodeGateway::Device dev;
        dev = m_devList.at(i);
        success = getController().systemCall("mknod " + dev.name + " c " +
                                                dev.major + " " + dev.minor);
        if (success) {
            success = getController().systemCall("chmod " +
                                                    dev.mode + " " + dev.name );
        } else {
            log_error() << "Failed to create device " << dev.name;
        }
    }

    return success;
}
