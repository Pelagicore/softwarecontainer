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

#include "config/softwarecontainerconfig.h"

#include <memory>

namespace softwarecontainer {

class DynamicContainerOptions
{
public:
    /*
     * @brief creates a SoftwareContainerConfig with the dynamic options set
     *
     * @param conf a SoftwareContainerConfig with static settings
     * @return a SoftwareContainerConfig pointer where the dynamic options that this object
     *         holds are set
     */
    std::unique_ptr<SoftwareContainerConfig> toConfig(const SoftwareContainerConfig &conf) const;

    /*
     * Setter and getter for the dynamic value on whether or not to enable
     * buffered write to disk for the container.
     */
    void setWriteBufferEnabled(bool enabled);
    bool writeBufferEnabled() const;

    /**
     * @brief Setter for the temporaryFileSystemWriteBufferEnableds variable used to tell the container
     * if it shall mount a separate tmpfs on top of the temp directory
     */
    void setTemporaryFileSystemWriteBufferEnabled(bool enabled);
    /**
     * @brief Getter for the temporaryFileSystemWriteBufferEnableds variable
     */
    bool temporaryFileSystemWriteBufferEnabled() const;

    /**
     * @brief Setter for the temporaryFileSystemSize which is used to tell the system the size of the
     * tmpfs being mounted on the temp directory.
     * @param size in bytes of the filesystem
     */
    void setTemporaryFileSystemSize(unsigned int size);
    /**
     * @brief Getter for the temporaryFileSystemSize variable.
     * @return the size
     */
    unsigned int temporaryFileSystemSize() const;

private:
    bool m_writeBufferEnabled = false;
    bool m_temporaryFileSystemWriteBufferEnabled = false;
    unsigned int m_temporaryFileSystemSize;
};

} // namespace softwarecontainer
