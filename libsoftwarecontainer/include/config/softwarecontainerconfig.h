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

#pragma once

#include <string>

namespace softwarecontainer {

/**
 * @class SoftwareContainerConfig
 *
 * @brief Contains all values that should be passed to SoftwareContainer on creation
 *
 * This class should be created with values that are from the static configs, i.e. the
 * configs that are known and fetched through Config (usually by SoftwareContainerAgent)
 * during startup.
 *
 * This class is also used to carry dynamic configs to SoftwareContainer and these can
 * be set through setters on this class. However, this class is not intended to be used
 * to set values dynamically in any way by SoftwareContainer, so it should be received
 * as const.
 */

class SoftwareContainerConfig
{
public:
    SoftwareContainerConfig() {}

    SoftwareContainerConfig(
#ifdef ENABLE_NETWORKGATEWAY
                            const bool shouldCreateBridge,
                            const std::string &bridgeDevice,
                            const std::string &bridgeIPAddress,
                            const std::string &bridgeNetmask,
                            const int bridgeNetmaskBitLength,
                            const std::string &bridgeNetAddr,
#endif // ENABLE_NETWORKGATEWAY
                            const std::string &containerConfigPath,
                            const std::string &sharedMountsDir,
                            unsigned int containerShutdownTimeout);

    ~SoftwareContainerConfig() {}

    /*
     * Setters for the values that are not part of the static configs
     */
    void setEnableWriteBuffer(bool enabledFlag);
    void setEnableTemporaryFileSystemWriteBuffers(bool enabled);
    void setTemporaryFileSystemSize(unsigned int size);

    /*
     * Getters for values that are set on creation only, i.e. these originate from the
     * static configs and should not be "re-set" after creation of this class.
     */
#ifdef ENABLE_NETWORKGATEWAY
    bool shouldCreateBridge() const;
    std::string bridgeDevice() const;
    std::string bridgeIPAddress() const;
    std::string bridgeNetmask() const;
    int bridgeNetmaskBitLength() const;
    std::string bridgeNetAddr() const;
#endif // ENABLE_NETWORKGATEWAY

    std::string containerConfigPath() const;
    std::string sharedMountsDir() const;
    unsigned int containerShutdownTimeout() const;

    /*
     * Getters for values that do not originate from the static configs and thus might be
     * set after creation of this class, i.e. these can be set with setters
     */
    bool writeBufferEnabled() const;
    bool temporaryFileSystemWriteBufferEnableds() const;
    unsigned int temporaryFileSystemSize() const;

private:
#ifdef ENABLE_NETWORKGATEWAY
    bool m_shouldCreateBridge;
    std::string m_bridgeDevice;
    std::string m_bridgeIPAddress;
    std::string m_bridgeNetmask;
    int m_bridgeNetmaskBitLength;
    std::string m_bridgeNetAddr;
#endif // ENABLE_NETWORKGATEWAY

    std::string m_containerConfigPath;
    std::string m_sharedMountsDir;
    unsigned int m_containerShutdownTimeout;

    bool m_writeBufferEnabled;
    bool m_temporaryFileSystemWriteBufferEnableds;
    unsigned int m_temporaryFileSystemSize;
};

} // namespace softwarecontainer
