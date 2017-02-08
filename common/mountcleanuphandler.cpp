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

#include "mountcleanuphandler.h"
#include <sys/mount.h>

namespace softwarecontainer {

MountCleanUpHandler::MountCleanUpHandler(const std::string &path)
{
    m_path = path;
}

bool MountCleanUpHandler::clean()
{
    // Lazy unmount. Should be the equivalent of the "umount -l" command.
    if (umount2(m_path.c_str(), MNT_DETACH) == 0) {
        log_debug() << "Unmounted " << m_path;
        return true;
    } else {
        log_warn() << "Can't unmount " << m_path << " . Error :" << strerror(errno) << ". Trying to force umount";
        if (umount2(m_path.c_str(), MNT_FORCE) != 0) {
            log_warn() << "Can't force unmount " << m_path << " . Error :" << strerror(errno);
            return false;
        }
        log_debug() << "Managed to force unmount " << m_path;
        return true;
    }
}

const std::string MountCleanUpHandler::queryName()
{
    return "";
}

} // namespace softwarecontainer
