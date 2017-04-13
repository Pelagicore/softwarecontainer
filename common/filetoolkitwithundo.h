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

#include "mountcleanuphandler.h"
#include "createdir.h"
#include "softwarecontainer-common.h"

namespace softwarecontainer {

class FileToolkitWithUndo
{
    LOG_DECLARE_CLASS_CONTEXT("CLEA", "File toolkit");

public:
    ~FileToolkitWithUndo();

    /**
     * @brief bindMount Bind mount a src directory to another position dst.
     * @param src Path to mount from
     * @param dst Path to mount to
     * @param readOnly Make the bind mount destination read only
     * @param writeBufferEnabled Enable write buffers on the bind mount.
     * @return true on success, false on failure
     */
    bool bindMount(const std::string &src,
                   const std::string &dst,
                   const std::string &tmpContainerRoot,
                   bool readOnly,
                   bool writeBufferEnabled=false);

    /**
     * @brief tmpfsMount Mount a tmpfs in the dst path and limit size of the
     *  tmpfs to maxSize
     * @param dst The destination to mount a tmpfs on
     * @param maxSize The max size of the tmpfs being mounted
     * @return
     */
    bool tmpfsMount(const std::string dst, const int maxSize);

protected:
    /*
     * @brief Writes to a file (and optionally create it)
     */
    bool writeToFile(const std::string &path, const std::string &content);

    /*
     * @brief Creates a file cleanup handler for a specific file.
     *
     * This is useful if one creates a file in some other way and want it deleted later.
     */
    void markFileForDeletion(const std::string &path);

    /**
     * @brief overlayMount Mount a directory with an overlay on top of it. An overlay protects
     *  the lower filesystem from writes by writing to the upper file system through the work
     *  directory.
     * @param lower The lower file system, this will be read only.
     * @param upper The upper file system, this can be a tmpfs/ramfs of some kind. This is where
     *  final writes wind up
     * @param work This is a work directory, preferably a tmpfs/ramfs of some kind. This is where
     *  writes wind up temporarily.
     * @param dst Where the overlay filesystem will be mounted.
     * @return true on success, false on failure
     */
    bool overlayMount(const std::string &lower,
                      const std::string &upper,
                      const std::string &work,
                      const std::string &dst);

    /**
     * @brief syncOverlayMount Copy the directory structure from upper layer to the lower layer
     * @param lower The lower layer used in an overlay file system.
     * @param upper The upper layer in an overlay file system.
     * @return true on success, false on failure
     */
    bool syncOverlayMount(const std::string &lower,
                          const std::string &upper);

    /**
     * @brief createSharedMountPoint Make the mount point shared, ie new mount points created in
     *  one bind mount will also be created in the other mount point.
     * @param path The mount path to make shared.
     * @return true on success, false on failure
     */
    bool createSharedMountPoint(const std::string &path);

    /**
     * @brief checks whether given path is already added to clean up handlers or not
     *
     * This function will be called only before adding new CleanUpHandler for FileCleanUpHandler
     * and DirectoryCleanUpHandler. The reason behind this, only indicated CleanUpHandlers are
     * related with file/directory removal operations.
     *
     * @param a string path name to check
     * @return true if the path is already exist, false otherwise
     */
    bool pathInList(const std::string path);

    /**
     * @brief m_cleanupHandlers A vector of cleanupHandlers added during the lifetime of the
     *  FileToolKitWithUndo that will be run from the destructor.
     */
    std::vector<std::unique_ptr<CleanUpHandler>> m_cleanupHandlers;

    /**
     * @brief m_createDirList A vector of CreateDir classes. This class handles directory cleaning
     * operations when either interfered errors or when its destructor runs.
     */
    std::vector<std::unique_ptr<CreateDir>> m_createDirList;
};

} // namespace softwarecontainer
