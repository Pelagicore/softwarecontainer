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

#include <cleanuphandler.h>

namespace softwarecontainer {

/**
 * @brief The OverlaySyncCleanupHandler class is used to copy files on cleanup and can be added to the
 * CleanupHandler stack in the FileToolkitWithUndo class. On destruction onf the
 * FileToolKitWithUndo, the src will be copied to dst recursively.
 */
class OverlaySyncCleanupHandler : public CleanUpHandler
{
public:
    /**
     * @brief OverlaySyncCleanupHandler Constructor of the class.
     * @param src Source to copy the files from
     * @param dst Destination of the files.
     */
    OverlaySyncCleanupHandler(std::string src, std::string dst);

    /**
     * @brief clean Performs the cleanup handling.
     * @return ReturnCode::SUCCESS on success
     * @return ReturnCode::FAILURE on failure
     */
    ReturnCode clean() override;

    /**
     * @brief queryName is needed to query member name yet its full functionality is not needed for
     * this class.
     *
     * Since no directory removal operation occurs within OverlaySyncCleanupHandler, it is
     * irrelevant when the code checks whether the directory is in the list or not since
     * the purpose of this function is to be used for cleaning path.
     *
     * @return an empty string
     */
    const std::string queryName() override;
private:
    std::string m_src;
    std::string m_dst;

};

} // namespace softwarecontainer
