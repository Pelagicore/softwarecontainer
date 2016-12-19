
/*
 * Copyright (C) 2016 Pelagicore AB
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

DirectoryCleanUpHandler::DirectoryCleanUpHandler(const std::string &path)
{
    m_path = path;
}

ReturnCode DirectoryCleanUpHandler::clean()
{
    if (!existsInFileSystem(m_path)) {
        log_warning() << "Folder " << m_path << " does not exist";
        return ReturnCode::SUCCESS;
    }

    if (rmdir(m_path.c_str()) == 0) {
        log_debug() << "rmdir'd " << m_path;
        return ReturnCode::SUCCESS;
    } else {
        log_error() << "Can't rmdir " << m_path << " . Error :" << strerror(errno);
        return ReturnCode::FAILURE;
    }
}

const std::string DirectoryCleanUpHandler::queryName()
{
    return m_path;
}
