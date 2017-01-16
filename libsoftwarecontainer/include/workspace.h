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

#include "filetoolkitwithundo.h"
#include "softwarecontainererror.h"

namespace softwarecontainer {

/**
 * @brief Holds information about the container workspace
 */
class Workspace :
    private FileToolkitWithUndo
{
    LOG_DECLARE_CLASS_CONTEXT("PCLW", "SoftwareContainer library workspace");

public:

    /**
     * @brief Creates a workspace.
     *
     * @throws SoftwareContainerError if there is an error during the setup.
     *
     * @param enableWriteBuffer Enable writebuffers on mountpoints
     * @param containerRootDir The path at which the container resides in the host system
     * @param containerConfigPath The path to the configuration lxc
     * @param containerShutdownTimeout The timeout time for the softwarecontainer
     *
     * TODO: Remove the abouve param docs if the resulting workspace design
     *       means we use the default constructor.
     */
    Workspace();

    ~Workspace();

    /**
     * @brief Check if the workspace is present and create it if needed
     */
    ReturnCode checkWorkspace();

    bool m_enableWriteBuffer;
    std::string m_containerRootDir;
    std::string m_containerConfigPath;
    unsigned int m_containerShutdownTimeout;
};

} // namespace softwarecontainer
