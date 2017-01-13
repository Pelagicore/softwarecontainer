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
 * @brief The RecursiveCopy class is a singleton class used to copy files recursively from a
 * source to a destination.
 */
class RecursiveCopy {
    LOG_DECLARE_CLASS_CONTEXT("RECO", "Recursive Copy");
public:
    /**
     * @brief getInstance Gets the RecursiveCopy instance.
     * @return The RecursiveCopy instance.
     */
    static RecursiveCopy &getInstance();

    /**
     * Disallow copy and operator constructors.
     */
    RecursiveCopy(RecursiveCopy &s) = delete;
    void operator=(RecursiveCopy const &s) = delete;


    /**
     * @brief copy Copy files from src to dst
     *
     * The function is protected by a mutex because it uses some non thread-safe C functions.
     * @param src The source path to copy from
     * @param dst The destination path to copy to
     * @return ReturnCode::SUCCESS on success
     * @return ReturnCode::FAILURE on failure
     */
    ReturnCode copy(std::string src, std::string dst);

private:
    RecursiveCopy();
    ~RecursiveCopy();

    /**
     * @brief m_copyLock lock used to protect global variables in the copy() method.
     */
    std::mutex m_copyLock;
};

} // namespace softwarecontainer
