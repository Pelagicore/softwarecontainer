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

#include <sys/stat.h>
#include <sys/mount.h>

#include "filetoolkitwithundo.h"

#include "directorycleanuphandler.h"
#include "filecleanuphandler.h"
#include "mountcleanuphandler.h"
#include "overlaysynccleanuphandler.h"

#include "recursivecopy.h"

namespace softwarecontainer {

FileToolkitWithUndo::~FileToolkitWithUndo()
{
    bool success = true;
    // Clean up all created directories, files, and mount points

    while (!m_cleanupHandlers.empty()) {
        CleanUpHandler *c = m_cleanupHandlers.back();
        m_cleanupHandlers.pop_back();

        if (!c->clean()) {
            success = false;
        }

        delete c;
    }

    if(!success) {
        log_error() << "One or more cleanup handlers returned error status, please check the log";
    }
}

bool FileToolkitWithUndo::createParentDirectory(const std::string &path)
{
    log_debug() << "Creating parent directories for " << path;
    std::string parent = parentPath(path);

    if (!createDirectory(parent)) {
        log_error() << "Could not create directory " << parent;
        return false;
    }

    return true;
}

bool FileToolkitWithUndo::createDirectory(const std::string &path)
{
    log_debug() << "createDirectory(" << path << ") called";
    if (isDirectory(path)) {
        return true;
    }

    if(!createParentDirectory(path)) {
        log_error() << "Couldn't create parent directory for " << path;
        return false;
    }

    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        log_error() << "Could not create directory " << path << ": " << strerror(errno);
        return false;
    }

    if (!pathInList(path)) {
        m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(path));
    }
    log_debug() << "Created directory " << path;

    return true;
}

std::string FileToolkitWithUndo::tempDir(std::string templ)
{
    char *dir = const_cast<char*>(templ.c_str());
    dir = mkdtemp(dir);
    if (dir == nullptr) {
        log_warning() << "Failed to create buffered Directory: " << strerror(errno);
        return nullptr;
    }

    if (!pathInList(templ)) {
        m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(templ));
    }

    return std::string(dir);
}

bool FileToolkitWithUndo::bindMount(const std::string &src,
                                    const std::string &dst,
                                    const std::string &tmpContainerRoot,
                                    bool readOnly,
                                    bool enableWriteBuffer)
{
    unsigned long flags =  0;
    std::string fstype;
    int mountRes;

    if (!existsInFileSystem(src)) {
        log_error() << src << " does not exist on the host, can not bindMount";
        return false;
    }

    if (!existsInFileSystem(dst)) {
        log_error() << dst << " does not exist on the host, can not bindMount";
        return false;
    }

    log_debug() << "Bind-mounting " << src << " in " << dst << ", flags: " << flags;

    if(enableWriteBuffer && isDirectory(src)) {
        std::string upperDir , workDir;

        // In case the tmpContainerRoot is set to nothing we need to create a
        // good bindmount directory.
        if (tmpContainerRoot == "") {
            upperDir = tempDir("/tmp/sc-bindmount-upper-XXXXXX");
            workDir = tempDir("/tmp/sc-bindmount-work-XXXXXX");
        } else {
            upperDir = tempDir(tmpContainerRoot + "/bindmount-upper-XXXXXX");
            workDir = tempDir(tmpContainerRoot + "/bindmount-work-XXXXXX");
        }
        fstype.assign("overlay");

        std::ostringstream os;
        os << "lowerdir=" << src << ",upperdir=" << upperDir << ",workdir=" << workDir;

        log_debug() << "enableWriteBuffer, config: " << os.str();

        mountRes = mount("overlay", dst.c_str(), fstype.c_str(), flags, os.str().c_str());
        log_debug() << "mountRes: " << mountRes;
    } else {
        const void *data = nullptr;
        flags = MS_BIND;
        mountRes = mount(src.c_str(), dst.c_str(), fstype.c_str(), flags, data);
    }

    if (mountRes == 0) {
        log_verbose() << "Bind-mounted folder " << src << " in " << dst;
        m_cleanupHandlers.push_back(new MountCleanUpHandler(dst));
    } else {
        log_error() << "Could not mount into container: src=" << src
                    << " , dst=" << dst << " err=" << strerror(errno);
        return false;
    }

    if (readOnly && !enableWriteBuffer) {
        const void *data = nullptr;

        flags = MS_REMOUNT | MS_RDONLY | MS_BIND;

        log_debug() << "Re-mounting read-only" << src << " in "
                    << dst << ", flags: " << flags;
        mountRes = mount(src.c_str(), dst.c_str(), fstype.c_str(), flags, data);
        if (mountRes != 0) {
            // Failure
            log_error() << "Could not re-mount " << src << " , read-only on "
                        << dst << " err=" << strerror(errno);
            return false;
        }
    }
    return true;
}

bool FileToolkitWithUndo::overlayMount(const std::string &lower,
                                       const std::string &upper,
                                       const std::string &work,
                                       const std::string &dst)
{
    std::string fstype = "overlay";
    unsigned long flags = 0;

    if (!createDirectory(lower)
        || !createDirectory(upper)
        || !createDirectory(work)
        || !createDirectory(dst))
    {
        log_error() << "Failed to create lower/upper/work directory for overlayMount. lowerdir=" <<
                       lower << ", upperdir=" << upper << ", workdir=" << work;
        return false;
    }

    std::string mountoptions = logging::StringBuilder() << "lowerdir=" << lower
                                                        << ",upperdir=" << upper
                                                        << ",workdir=" << work;

    int mountRes = mount("overlay", dst.c_str(), fstype.c_str(), flags, mountoptions.c_str());

    if (mountRes == 0) {
        log_verbose() << "overlayMounted folder " << lower << " in " << dst;
        m_cleanupHandlers.push_back(new MountCleanUpHandler(dst));
        if (!pathInList(upper)) {
            m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(upper));
        }
        m_cleanupHandlers.push_back(new OverlaySyncCleanupHandler(upper, lower));
        if (!pathInList(work)) {
            m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(work));
        }
    } else {
        log_error() << "Could not mount into container: upper=" << upper
                    << ",lower=" << lower
                    << ",work=" << work
                    << " at dst=" << dst << " err=" << strerror(errno);
        return false;
    }

    return true;
}

bool FileToolkitWithUndo::syncOverlayMount(const std::string &lower,
                                           const std::string &upper)
{
    return RecursiveCopy::getInstance().copy(upper, lower);
}

bool FileToolkitWithUndo::createSharedMountPoint(const std::string &path)
{
    auto mountRes = mount(path.c_str(), path.c_str(), "", MS_BIND, nullptr);
    if (mountRes != 0) {
        log_error() << "Could not bind mount " << path << " to itself";
        return false;
    }

    mountRes = mount(path.c_str(), path.c_str(), "", MS_UNBINDABLE, nullptr);
    if (mountRes != 0) {
        log_error() << "Could not make " << path << " unbindable";
        return false;
    }

    mountRes = mount(path.c_str(), path.c_str(), "", MS_SHARED, nullptr);
    if (mountRes != 0) {
        log_error() << "Could not make " << path << " shared";
        return false;
    }

    m_cleanupHandlers.push_back(new MountCleanUpHandler(path));
    log_debug() << "Created shared mount point at " << path;

    return true;
}

bool FileToolkitWithUndo::pathInList(const std::string path)
{
    for (auto element : m_cleanupHandlers) {
        if (element->queryName() == path) {
            return true;
        }
    }
    return false;
}

bool FileToolkitWithUndo::writeToFile(const std::string &path, const std::string &content)
{
    if (!softwarecontainer::writeToFile(path, content)) {
        return false;
    }

    if (!pathInList(path)) {
        m_cleanupHandlers.push_back(new FileCleanUpHandler(path));
    }
    log_debug() << "Successfully wrote to " << path;
    return true;
}

void FileToolkitWithUndo::markFileForDeletion(const std::string &path)
{
    if (!pathInList(path)) {
        m_cleanupHandlers.push_back(new FileCleanUpHandler(path));
    }
}

} // namespace softwarecontainer
