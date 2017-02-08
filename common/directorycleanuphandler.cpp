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

#include "directorycleanuphandler.h"

namespace softwarecontainer {

DirectoryCleanUpHandler::DirectoryCleanUpHandler(const std::string &path)
{
    m_path = path;
}

bool DirectoryCleanUpHandler::clean()
{
    if (!existsInFileSystem(m_path)) {
        log_warning() << "Folder " << m_path << " does not exist";
        return true;
    }

    if (rmdir(m_path.c_str()) == 0) {
        log_debug() << "rmdir'd " << m_path;
        return true;
    } else {
        log_error() << "Can't rmdir " << m_path << " . Error :" << strerror(errno);
        return false;
    }
}

const std::string DirectoryCleanUpHandler::queryName()
{
    return m_path;
}

} // namespace softwarecontainer
