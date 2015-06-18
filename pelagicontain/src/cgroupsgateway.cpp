/*
 *   Copyright (C) 2015 Pelagicore AB
 *   All rights reserved.
 */

#include "jansson.h"

#include "pelagicontain-common.h"
#include "cgroupsgateway.h"

CgroupsGateway::CgroupsGateway()
    : Gateway(ID)
    , m_settings({})
    , m_hasBeenConfigured(false)
{
}

bool CgroupsGateway::activate()
{
    if (!m_hasBeenConfigured) {
        log_warning() << "activate was called on CgroupsGateway which has not been configured";
        return false;
    }

    ReturnCode success = ReturnCode::FAILURE;
    for (auto setting: m_settings) {
        success = getContainer().setCgroupItem(setting.first, setting.second);
        if (success != ReturnCode::SUCCESS) {
            log_error() << "Error activating Cgroups Gateway";
            break;
        }
    }

    return success == ReturnCode::SUCCESS ? true : false;
}

ReturnCode CgroupsGateway::readConfigElement(const JSonElement &element)
{
    if (!element.isValid()) {
        log_error() << "Error: invalid JSON data";
        return ReturnCode::FAILURE;
    }

    if (!element.isObject()) {
        log_error() << "Error: Elements must be JSON objects";
        return ReturnCode::FAILURE;
    }

    const json_t *data = element.root();
    json_t *setting;
    json_t *value;

    setting = json_object_get(data, "setting");
    if (!json_is_string(setting)) {
        log_error() << "Error: setting is not a string";
        return ReturnCode::FAILURE;
    }
    std::string settingString = json_string_value(setting);

    value = json_object_get(data, "value");
    if (!json_is_string(value)) {
        log_error() << "Error, value is not a string";
        return ReturnCode::FAILURE;
    }

    std::string valueString = json_string_value(value);
    m_settings.insert( std::pair<std::string, std::string>(settingString, valueString) );
    return ReturnCode::SUCCESS;
}

