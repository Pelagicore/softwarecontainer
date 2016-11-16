
/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

#include "devicenodegateway.h"
#include <sys/stat.h>
#include <sys/types.h>


DeviceNodeGateway::DeviceNodeGateway() :
    Gateway(ID)
{
}

ReturnCode DeviceNodeGateway::readConfigElement(const json_t *element)
{
    DeviceNodeGateway::Device dev;

    if (!JSONParser::read(element, "name", dev.name)) {
        log_error() << "Key \"name\" missing or not a string in json configuration";
        return ReturnCode::FAILURE;
    }

    JSONParser::read(element, "major", dev.major);
    JSONParser::read(element, "minor", dev.minor);
    JSONParser::read(element, "mode",  dev.mode);

    const bool majorSpecified = dev.major.length() > 0;
    const bool minorSpecified = dev.minor.length() > 0;

    if (majorSpecified | minorSpecified) {
        if (!checkIsStrValid(dev.major, "Major version", "Major version must be specified when minor is.")
            || !checkIsStrValid(dev.minor, "Minor version", "Minor version must be specified when major is.")
            || !checkIsStrValid(dev.mode, "Mode", "Mode has to be specified when major and minor is specified.")) {
            return ReturnCode::FAILURE;
        }
    }

    m_devList.push_back(dev);
    return ReturnCode::SUCCESS;
}

bool DeviceNodeGateway::checkIsStrValid(std::string intStr, std::string name, std::string notSpecified)
{
    if (intStr.length() == 0) {
        log_error() << notSpecified;
        return false;
    }

    int i = 0;
    if (!parseInt(intStr.c_str(), &i)) {
        log_error() << name << " must be represented as an integer.";
        return false;
    }

    return true;
}

bool DeviceNodeGateway::activateGateway()
{
    if (m_devList.empty()) {
        log_info() << "Activate was called when no devices has been configured.";
        return false;
    }

    for (auto &dev : m_devList) {
        log_info() << "Mapping device " << dev.name;

        if (dev.major.length() != 0) {

            int majorVersion, minorVersion, mode;
            parseInt(dev.major.c_str(), &majorVersion);
            parseInt(dev.minor.c_str(), &minorVersion);
            parseInt(dev.mode.c_str(), &mode);

            // mknod dev.name c dev.major dev.minor
            pid_t pid = INVALID_PID;
            getContainer()->executeInContainer([&] () {
                return mknod(dev.name.c_str(), S_IFCHR | mode,
                             makedev(majorVersion, minorVersion));
            }, &pid);

            if (waitForProcessTermination(pid) != 0) {
                log_error() << "Failed to create device " << dev.name;
                return false;
            }
        } else {
            // No major & minor numbers specified => simply map the device from
            // the host into the container
            getContainer()->mountDevice(dev.name);

            if (dev.mode.length() != 0) {
                const int mode = std::atoi(dev.mode.c_str());

                pid_t pid = INVALID_PID;
                getContainer()->executeInContainer([&] () {
                    return chmod(dev.name.c_str(), mode);
                }, &pid);

                if (waitForProcessTermination(pid) != 0) {
                    log_error() << "Could not 'chmod " << dev.mode
                                << "' the mounted device " << dev.name;
                    return false;
                }
            }
        }
    }

    return true;
}

bool DeviceNodeGateway::teardownGateway()
{
    return true;
}
