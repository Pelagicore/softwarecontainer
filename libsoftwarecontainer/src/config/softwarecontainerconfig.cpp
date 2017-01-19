/*
 * Copyright (C) 2016-2017 Pelagicore AB
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

#include <string>

#include "config/softwarecontainerconfig.h"


namespace softwarecontainer {

SoftwareContainerConfig::SoftwareContainerConfig(const std::string &bridgeIp,
                                                 const std::string &containerConfigPath,
                                                 const std::string &containerRootDir,
                                                 int netmaskBitLength,
                                                 unsigned int containerShutdownTimeout):
    m_bridgeIp(bridgeIp),
    m_containerConfigPath(containerConfigPath),
    m_containerRootDir(containerRootDir),
    m_netmaskBitLength(netmaskBitLength),
    m_containerShutdownTimeout(containerShutdownTimeout)
{
}

void SoftwareContainerConfig::setEnableWriteBuffer(bool enabledFlag)
{
    m_enableWriteBuffer = enabledFlag;
}

std::string SoftwareContainerConfig::bridgeIp() const
{
    return m_bridgeIp;
}

std::string SoftwareContainerConfig::containerConfigPath() const
{
    return m_containerConfigPath;
}

std::string SoftwareContainerConfig::containerRootDir() const
{
    return m_containerRootDir;
}

int SoftwareContainerConfig::netmaskBitLength() const
{
    return m_netmaskBitLength;
}

unsigned int SoftwareContainerConfig::containerShutdownTimeout() const
{
    return m_containerShutdownTimeout;
}

bool SoftwareContainerConfig::enableWriteBuffer() const
{
    return m_enableWriteBuffer;
}


} // namespace softwarecontainer
