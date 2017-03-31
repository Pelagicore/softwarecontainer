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

#include "directorycleanuphandler.h"

namespace softwarecontainer {

class CreateDir {
    LOG_DECLARE_CLASS_CONTEXT("CD", "Create Directory");
public:
    /**
     * @brief createDirectory Create a directory, and if successful append it
     *  to a list of dirs to be deleted in the dtor. Since nestled dirs will
     *  need to be deleted in reverse order to creation insert to the beginning
     *  of the list.
     * @param path Path of directory to be created
     * @return true on success, false on failure
     */
    bool createDirectory(const std::string path);

    /**
     * @brief rollBack calls clean method for all members of m_createDirectoryCleanupHandlers.
     *
     * This function is created as a helper to callers of createDirectory functions. It is strongly
     * recommended that this functions should be called if createDirectory function returns false.
     *
     * @return true on success, false on failure
     */
    bool rollBack();

    /*@brief clear removes all elements from the m_createDirectoryCleanupHandlers
     *
     * */
    void clear();

    /**
     * @brief tempDir Creates a uniquely named temporary directory according to templatePath.
     *
     * @warning The temporary path will be destroyed when the instance of FileToolkitWithUndo
     *  is destroyed.
     *
     * @param templ a template Path used to create the path of the temporary directory, including
     *  XXXXXX which will be replaced with a unique ID for the temporary directory
     *
     * @throws SoftwareContainerError if template path is not appropriate.
     * @return A string path pointing to the newly created temporary directory.
     */
    std::string tempDir(std::string templatePath);

    /**
     * @brief m_tempFileCleaners holds cleanup handler for each created temporary directory.
     * The main purpose of this is to rollback in any errors while bind-mounting. On success
     * m_tempFileCleaners will be moved to cleanup handlers of FileToolkitWithUndo
     */
    std::vector<std::unique_ptr<DirectoryCleanUpHandler>> m_tempFileCleaners;
private :
    /**
     * @brief createParentDirectory Recursively tries to create the directory pointed to by path.
     * @param path The directory path to be created.
     * @return true on success, false on failure
     */
    bool createParentDirectory(const std::string path);

    /**
     * @brief m_createDirectoryCleanupHandlers holds cleanup handler for each created directory.
     * The main purpose of this is to rollback in any errors.
     */
    std::vector<std::unique_ptr<DirectoryCleanUpHandler>> m_rollbackCleaners;

    /**
     * @brief checks whether path is already added to m_cleanupHandlers or not
     * @param a string path name to check
     * @return true if the path is already exist, false otherwise
     */
    bool pathInList(const std::string path);
};

}
