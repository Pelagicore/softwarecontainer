/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "generators.h"

#include "devicenodegateway.h"


DeviceNodeGateway::DeviceNodeGateway() :
    Gateway(ID)
{
}

ReturnCode DeviceNodeGateway::readConfigElement(const JSonElement &element)
{
    DeviceNodeGateway::Device dev;

    element.read("name", dev.name);
    element.read("major", dev.major);
    element.read("minor", dev.minor);
    element.read("mode", dev.mode);

    m_devList.push_back(dev);

    return ReturnCode::SUCCESS;
}


bool DeviceNodeGateway::activateGateway()
{
    for (auto &dev : m_devList) {
        log_info() << "Mapping device " << dev.name;

        if (dev.major.length() != 0) {
            auto success = executeInContainer("mknod " + dev.name + " c " + dev.major + " " + dev.minor);
            if (!isError(success)) {
                success = executeInContainer("chmod " + dev.mode + " " + dev.name);
            } else {
                log_error() << "Failed to create device " << dev.name;
                return false;
            }
        } else {
            // No major & minor numbers specified => simply map the device from the host into the container
            getContainer().mountDevice(dev.name);

            // TODO : check if it is fine to authorize write access systematically
            std::string cmd = StringBuilder() << "chmod o+rwx " << dev.name;
            getContainer().executeInContainer(cmd);
        }
    }

    m_state = GatewayState::ACTIVATED;
    return true;
}

bool DeviceNodeGateway::teardownGateway()
{
    return true;
}
