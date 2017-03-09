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
#include "softwarecontainer-common.h"

namespace softwarecontainer {

/**
 * @brief The RecursiveDelete class is a singleton class used to delete files recursively in a
 * directory.
 */
class RecursiveDelete {
    LOG_DECLARE_CLASS_CONTEXT("RECO", "Recursive Delete");
public:
    /**
     * @brief getInstance Gets the RecursiveDelete instance.
     * @return The RecursiveDelete instance.
     */
    static RecursiveDelete &getInstance();

    /**
     * Disallow copy and operator constructors.
     */
    RecursiveDelete(RecursiveDelete &s) = delete;
    void operator=(RecursiveDelete const &s) = delete;


    /**
     * @brief delete Delete files from directory
     *
     * The function is protected by a mutex because it uses some non thread-safe C functions.
     * @param dir The source path to deletefrom
     * @return true on success
     * @return false on failure
     */
    bool del(std::string dir);

private:
    RecursiveDelete();
    ~RecursiveDelete();

    /**
     * @brief m_deleteLock lock used to protect global variables in the delete() method.
     */
    std::mutex m_deleteLock;
};

} // namespace softwarecontainer
