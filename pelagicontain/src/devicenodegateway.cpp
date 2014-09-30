/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "generators.h"
#include "jansson.h"

#include "devicenodegateway.h"

DeviceNodeGateway::DeviceNodeGateway() :
    Gateway() {
}

bool DeviceNodeGateway::setConfig(const std::string &config) {
    JSonParser parser(config);

    bool success = true;

    log_debug() << "DeviceNodeGateway::setConfig called";

    // Get string
    json_t *devices = json_object_get(parser.root(), "devices");

    if ( !json_is_array(devices) ) {
        log_error() << "Value is not an array";
        success = false;
    } else {
        std::vector<Device> newDevList = parseDeviceList(devices, success);
        if (success)
            m_devList = newDevList;
    }

    return success;
}

std::vector<DeviceNodeGateway::Device>
DeviceNodeGateway::parseDeviceList(json_t *list, bool &ok) {
    ok = true;
    std::vector<DeviceNodeGateway::Device> dev_list;
    for (size_t i = 0; i < json_array_size(list); i++) {
        DeviceNodeGateway::Device dev;
        std::string *fields[] = {&dev.name, &dev.major, &dev.minor, &dev.mode};
        const char *fieldsStr[] = {"name", "major", "minor", "mode"};
        uint numFields = 4;

        json_t *device = json_array_get(list, i);
        if ( ok && !json_is_object(device) ) {
            log_error("Expected JSON device object, found something else");
            ok = false;
        }

        for (size_t j = 0; ok && j < numFields; j++) {
            json_t *value = 0;
            value = json_object_get(device, fieldsStr[j]);
            if ( ok && !json_is_string(value) ) {
                log_error() << "Key '" << fieldsStr[j] << "' is not a member of device object!";
                ok = false;
            }
            if ( json_string_value(value) ) {
                *fields[j] = std::string( json_string_value(value) );
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

bool DeviceNodeGateway::activate() {
    for(auto &dev : m_devList) {
        if (dev.major.length() != 0) {
            auto success = systemCall("mknod " + dev.name + " c " + dev.major + " " + dev.minor);
            if ( !isError(success) ) {
                success = systemCall("chmod " + dev.mode + " " + dev.name);
            } else {
                log_error() << "Failed to create device " << dev.name;
                return false;
            }
        } else {
            // No major & minor numbers specified => simply map the device from the host into the container
            getContainer().mountDevice(dev.name);
        }
    }

    return true;
}
