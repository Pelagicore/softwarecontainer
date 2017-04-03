/*
 * Copyright (C) 2017 Pelagicore AB
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

#include <sys/stat.h>

#include "devicenode.h"
#include "functionjob.h"

namespace softwarecontainer {

Device::Device(std::string name, int mode) :
        m_name(name),
        m_mode(mode),
        m_isConfigured(false)
{ }

Device::Device(const Device &dev) :
        m_name(dev.m_name),
        m_mode(dev.m_mode),
        m_isConfigured(false)
{ }

bool Device::parse(const json_t *element)
{

    m_name = "";
    if (!JSONParser::read(element, "name", m_name)) {
        log_error() << "Key \"name\" missing or not a string in json configuration";
        return false;
    }

    m_mode = -1;

    // Mode is optional, i.e. do not do anything if it is not specified.
    if (nullptr != json_object_get(element, "mode")) {
        const bool modeParses = JSONParser::read(element, "mode", m_mode);

        if (!modeParses) {
            log_error() << "Mode specified with bad format";
            return false;
        }
    }

    return true;
}

bool Device::activate(std::shared_ptr<ContainerAbstractInterface> container)
{
    if (!m_isConfigured) {
        log_info() << "Mapping device " << m_name;
        // Mount device in container
        if (!container->mountDevice(m_name)) {
            log_error() << "Unable to mount device " << m_name;
            return false;
        }

        // If mode is specified, try to set mode for the mounted device.
        if (m_mode != -1) {
            FunctionJob job(container, [&] () {
                return chmod(m_name.c_str(), m_mode);
            });
            job.start();
            job.wait();
            if (job.isError()) {
                log_error() << "Could not 'chmod " << m_mode
                            << "' the mounted device " << m_name;
                return false;
            }
        }
        m_isConfigured = true;
    }
    return true;
}

void Device::calculateDeviceMode(const int appliedMode)
{
    int modeResult = m_mode;

    if (m_mode != appliedMode) {
        //apply more permissive option for owner
        ((appliedMode / 100) >= (m_mode / 100)) ?
                modeResult = (appliedMode / 100) * 100 : modeResult = (m_mode / 100) * 100;
        //apply more permissive option for group
        (((appliedMode / 10) % 10) >= ((m_mode / 10) % 10)) ?
                modeResult += (((appliedMode / 10) % 10) * 10) : modeResult += (((m_mode / 10) % 10) * 10);
        //apply more permissive option for others
        ((appliedMode % 10) >= (m_mode % 10)) ?
                modeResult += (appliedMode % 10) :  modeResult += (m_mode % 10);

    }

    if (modeResult > m_mode) {
        m_isConfigured = false;
        m_mode = modeResult;
    }
}

const std::string Device::getName()
{
    return m_name;
}

int Device::getMode()
{
    return m_mode;
}

bool Device::getIsconfigured ()
{
    return m_isConfigured;
}

void Device::setMode(int mode)
{
    if ((mode/100) < 8 && (mode%100)/10 < 8 && (mode%10) < 8) {
        m_mode = mode;
    }
}



} // namespace softwarecontainer
