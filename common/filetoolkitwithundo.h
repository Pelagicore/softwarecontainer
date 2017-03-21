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

#include "cleanuphandler.h"
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
     * @param enableWriteBuffer Enable write buffers on the bind mount.
     * @return true on success, false on failure
     */
    bool bindMount(const std::string &src,
                   const std::string &dst,
                   bool readOnly,
                   bool enableWriteBuffer=false);


    /**
     * @brief tempDir Creates a temporary directory at templatePath.
     * @warning The temporary path will be destroyed when the instance of FileToolkitWithUndo
     *  is destroyed.
     * @param templ a template Path used to create the path of the temporary directory, including
     *  XXXXXX which will be replaced with a unique ID for the temporary directory
     * @return A string path pointing to the newly creted temporary directory.
     */
    std::string tempDir(std::string templatePath);
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
     * @brief createDirectory Create a directory, and if successful append it
     *  to a list of dirs to be deleted in the dtor. Since nestled dirs will
     *  need to be deleted in reverse order to creation insert to the beginning
     *  of the list.
     * @param path Path of directory to be created
     * @return true on success, false on failure
     */
    bool createDirectory(const std::string &path);

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
    std::vector<CleanUpHandler *> m_cleanupHandlers;
private :
    /**
     * @brief createParentDirectory Recursively tries to create the directory pointed to by path.
     * @param path The directory path to be created.
     * @return true on success, false on failure
     */
    bool createParentDirectory(const std::string &path);
};

} // namespace softwarecontainer
