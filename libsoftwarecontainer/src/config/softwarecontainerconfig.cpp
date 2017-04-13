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

SoftwareContainerConfig::SoftwareContainerConfig(
#ifdef ENABLE_NETWORKGATEWAY
                                                 const bool shouldCreateBridge,
                                                 const std::string &bridgeDevice,
                                                 const std::string &bridgeIPAddress,
                                                 const std::string &bridgeNetmask,
                                                 int bridgeNetmaskBitLength,
                                                 const std::string &bridgeNetAddr,
#endif // ENABLE_NETWORKGATEWAY
                                                 const std::string &containerConfigPath,
                                                 const std::string &sharedMountsDir,
                                                 unsigned int containerShutdownTimeout) :
#ifdef ENABLE_NETWORKGATEWAY
    m_shouldCreateBridge(shouldCreateBridge),
    m_bridgeDevice(bridgeDevice),
    m_bridgeIPAddress(bridgeIPAddress),
    m_bridgeNetmask(bridgeNetmask),
    m_bridgeNetmaskBitLength(bridgeNetmaskBitLength),
    m_bridgeNetAddr(bridgeNetAddr),
#endif
    m_containerConfigPath(containerConfigPath),
    m_sharedMountsDir(sharedMountsDir),
    m_containerShutdownTimeout(containerShutdownTimeout)
{
}

void SoftwareContainerConfig::setEnableWriteBuffer(bool enabledFlag)
{
    m_writeBufferEnabled = enabledFlag;
}

void SoftwareContainerConfig::setEnableTemporaryFileSystemWriteBuffers(bool enabled)
{
    m_temporaryFileSystemWriteBufferEnableds = enabled;
}

void SoftwareContainerConfig::setTemporaryFileSystemSize(unsigned int size)
{
    m_temporaryFileSystemSize = size;
}

std::string SoftwareContainerConfig::containerConfigPath() const
{
    return m_containerConfigPath;
}

std::string SoftwareContainerConfig::sharedMountsDir() const
{
    return m_sharedMountsDir;
}

unsigned int SoftwareContainerConfig::containerShutdownTimeout() const
{
    return m_containerShutdownTimeout;
}

bool SoftwareContainerConfig::writeBufferEnabled() const
{
    return m_writeBufferEnabled;
}

bool SoftwareContainerConfig::temporaryFileSystemWriteBufferEnableds() const
{
    return m_temporaryFileSystemWriteBufferEnableds;
}

unsigned int SoftwareContainerConfig::temporaryFileSystemSize() const
{
    return m_temporaryFileSystemSize;
}

#ifdef ENABLE_NETWORKGATEWAY

bool SoftwareContainerConfig::shouldCreateBridge() const
{
    return m_shouldCreateBridge;
}

std::string SoftwareContainerConfig::bridgeDevice() const
{
    return m_bridgeDevice;
}

std::string SoftwareContainerConfig::bridgeIPAddress() const
{
    return m_bridgeIPAddress;
}

std::string SoftwareContainerConfig::bridgeNetmask() const
{
    return m_bridgeNetmask;
}

int SoftwareContainerConfig::bridgeNetmaskBitLength() const
{
    return m_bridgeNetmaskBitLength;
}

std::string SoftwareContainerConfig::bridgeNetAddr() const
{
    return m_bridgeNetAddr;
}

#endif // ENABLE_NETWORKGATEWAY


} // namespace softwarecontainer
