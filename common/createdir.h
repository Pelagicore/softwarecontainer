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

#pragma once

#include "directorycleanuphandler.h"

namespace softwarecontainer {

/**
 * @brief The CreateDir class is responsible for creating new directories and removing them when it
 * is necessary.
 *
 * This class is created as a helper to FileToolkitWithUndo class. It adds value to
 * FileToolkitWithUndo class by creating rollback option on failures.
 */
class CreateDir {
    LOG_DECLARE_CLASS_CONTEXT("CD", "Create Directory");
public:
    ~CreateDir();

    /**
     * @brief createDirectory creates a directory according to given path.
     * The directory will be removed when the object is destroyed.
     *
     * @param path Path of directory to be created
     * @return true on success, false on failure
     */
    bool createDirectory(const std::string path);

    /**
     * @brief createTempDirectoryFromTemplate creates a uniquely named directory according to
     * templatePath.
     *
     * @warning The created directory will be destroyed when the object is destroyed
     *
     * @param templatePath is used to create the path of the temporary directory. The last six
     * characters of template must be XXXXXX and these are replaced with a string that makes the
     * directory name unique.
     *
     * @throws SoftwareContainerError if template path is not appropriate.
     * @return A string path pointing to the newly created temporary directory.
     */
    std::string createTempDirectoryFromTemplate(std::string templatePath);

private :
    /**
     * @brief createParentDirectory Recursively tries to create the directory pointed to by path.
     *
     * @param path The directory path to be created.
     * @return true on success, false on failure
     */
    bool createParentDirectory(const std::string path);

    /**
     * @brief holds cleanup handler for each created directory.
     * The main purpose of this is to rollback in any errors or when the object is
     */
    std::vector<DirectoryCleanUpHandler> m_rollbackCleaners;

    /**
     * @brief checks whether path is already added to m_rollbackCleaners or not
     *
     * @param a string path name to check
     * @return true if the path is already exist, false otherwise
     */
    bool pathInList(const std::string path);
};

}
