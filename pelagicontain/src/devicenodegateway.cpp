/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "generators.h"

#include "devicenodegateway.h"


DeviceNodeGateway::DeviceNodeGateway() :
    Gateway()
{
}

ReturnCode DeviceNodeGateway::readConfigElement(JSonElement &element)
{

    DeviceNodeGateway::Device dev;

    element.read("name", dev.name);
    element.read("major", dev.major);
    element.read("minor", dev.minor);
    element.read("mode", dev.mode);

    m_devList.push_back(dev);

    return ReturnCode::SUCCESS;
}


bool DeviceNodeGateway::activate()
{
    for (auto &dev : m_devList) {
        log_error() << "Mapping device " << dev.name;

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
