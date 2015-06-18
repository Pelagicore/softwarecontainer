/*
 *   Copyright (C) 2015 Pelagicore AB
 *   All rights reserved.
 */

#include "jansson.h"
#include "log.h"

#include "cgroupsgateway.h"


CgroupsGateway::CgroupsGateway(ControllerAbstractInterface &controllerInterface,
                                SystemcallAbstractInterface &systemcallInterface,
                                const std::string &containerName):
    Gateway(controllerInterface),
    m_systemcallInterface(systemcallInterface),
    m_containerName(containerName),
    m_hasBeenConfigured(false)
{

}

std::string CgroupsGateway::id()
{
    return "cgroups";
}

bool CgroupsGateway::setConfig(const std::string &config)
{
    m_settings = settingsFromConfig(config);
    m_hasBeenConfigured = true;
    return true;
}

bool CgroupsGateway::activate()
{
    if (!m_hasBeenConfigured) {
        log_warning() << "activate was called on CgroupsGateway which has not been configured";
        return false;
    }

    bool success = false;
    std::string commandIntro = "lxc-cgroup -n " + m_containerName + " ";

    for (auto setting: m_settings) {
        std::string command = commandIntro + setting;
        int success = m_systemcallInterface.makeCall(command);
        if (success == false) {
            log_error() << "Error activating Cgroups Gateway";
            break;
        }
    }

    return success;
}

bool CgroupsGateway::teardown()
{
    return true;
}

std::vector<std::string> CgroupsGateway::settingsFromConfig(const std::string &config)
{
    json_error_t error;
    json_t *root;
    json_t *data;
    json_t *setting;
    json_t *value;
    std::string settingString;
    std::string valueString;
    std::vector<std::string> settings;

    root = json_loads(config.c_str(), 0, &error);
    if (!root) {
        log_error() << "Error on line " << error.line << ": " << error.text;
        goto cleanup_parse_json;
    }
    if (!json_is_array(root)) {
        log_error() << "Error: root is not an array";
        json_decref(root);
    }

    for (unsigned i = 0; i < json_array_size(root); i++) {
        data = json_array_get(root, i);
        if (!json_is_object(data)) {
            log_error() << "Error: data is not an object, index: " << i;
            json_decref(root);
        }

        setting = json_object_get(data, "setting");
        if (!json_is_string(setting)) {
            log_error() << "Error: setting is not a string";
            json_decref(root);
        }
        settingString = json_string_value(setting);

        value = json_object_get(data, "value");
        if (!json_is_string(value)) {
            log_error() << "Error, value is not a string";
            json_decref(root);
        }
        valueString = json_string_value(value);

        std::string settingEntry = settingString + " " + valueString;

        settings.push_back(settingEntry);
    }

cleanup_parse_json:
    if (root) {
        json_decref(root);
    }

    return settings;
}
