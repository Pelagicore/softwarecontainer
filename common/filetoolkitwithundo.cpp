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

        if (isError(c->clean())) {
            success = false;
        }

        delete c;
    }

    if(!success) {
        log_error() << "One or more cleanup handlers returned error status, please check the log";
    }
}

ReturnCode FileToolkitWithUndo::createParentDirectory(const std::string &path)
{
    log_debug() << "Creating parent directories for " << path;
    std::string parent = parentPath(path);
    if (!isDirectory(parent) && !parent.empty()) {
        if(isError(createDirectory(parent))) {
            log_error() << "Could not create directory " << parent;
            return ReturnCode::FAILURE;
        }
    }
    return ReturnCode::SUCCESS;
}

ReturnCode FileToolkitWithUndo::createDirectory(const std::string &path)
{
    if (isDirectory(path)) {
        return ReturnCode::SUCCESS;
    }

    if(isError(createParentDirectory(path))) {
        log_error() << "Couldn't create parent directory for " << path;
        return ReturnCode::FAILURE;
    }

    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        log_error() << "Could not create directory " << path << " - Reason : " << strerror(errno);
        return ReturnCode::FAILURE;
    }

    if (!pathInList(path)) {
        m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(path));
    }
    log_debug() << "Created directory " << path;

    return ReturnCode::SUCCESS;
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

ReturnCode FileToolkitWithUndo::bindMount(const std::string &src,
                                          const std::string &dst,
                                          bool readOnly,
                                          bool enableWriteBuffer)
{
    unsigned long flags = MS_BIND;
    std::string fstype;
    const void *data = nullptr;
    int mountRes;

    if (!existsInFileSystem(src)) {
        log_error() << src << " does not exist on the host, can not bindMount";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Bind-mounting " << src << " in " << dst << ", flags: " << flags;

    if(enableWriteBuffer) {
        std::string upperDir = tempDir("/tmp/sc-bindmount-upper-XXXXXX");
        std::string workDir = tempDir("/tmp/sc-bindmount-work-XXXXXX");
        fstype.assign("overlay");

        std::ostringstream os;
        os << "lowerdir=" << src << ",upperdir=" << upperDir << ",workdir=" << workDir;
        data = os.str().c_str();

        log_debug() << "enableWriteBuffer, config: " << os.str();

        mountRes = mount("overlay", dst.c_str(), fstype.c_str(), flags, data);
    } else {
        mountRes = mount(src.c_str(), dst.c_str(), fstype.c_str(), flags, data);
    }

    if (mountRes == 0) {
        log_verbose() << "Bind-mounted folder " << src << " in " << dst;
        m_cleanupHandlers.push_back(new MountCleanUpHandler(dst));
    } else {
        log_error() << "Could not mount into container: src=" << src
                    << " , dst=" << dst << " err=" << strerror(errno);
        return ReturnCode::FAILURE;
    }

    if (readOnly) {
        flags = MS_REMOUNT | MS_RDONLY | MS_BIND;

        log_debug() << "Re-mounting read-only" << src << " in "
                    << dst << ", flags: " << flags;
        mountRes = mount(src.c_str(), dst.c_str(), fstype.c_str(), flags, data);
        if (mountRes != 0) {
            // Failure
            log_error() << "Could not re-mount " << src << " , read-only on "
                        << dst << " err=" << strerror(errno);
            return ReturnCode::FAILURE;
        }
    }
    return ReturnCode::SUCCESS;
}

ReturnCode FileToolkitWithUndo::overlayMount(
        const std::string &lower,
        const std::string &upper,
        const std::string &work,
        const std::string &dst)
{
    std::string fstype = "overlay";
    unsigned long flags = MS_BIND;

    if (isError(createDirectory(lower))
        || isError(createDirectory(upper))
        || isError(createDirectory(work))
        || isError(createDirectory(dst)))
    {
        log_error() << "Failed to create lower/upper/work directory for overlayMount. lower=" <<
                       lower << ", upper=" << upper << ", work=" << work;
        return ReturnCode::FAILURE;
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
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

ReturnCode FileToolkitWithUndo::syncOverlayMount(
        const std::string &lower,
        const std::string &upper)
{
    return RecursiveCopy::getInstance().copy(upper, lower);
}

ReturnCode FileToolkitWithUndo::createSharedMountPoint(const std::string &path)
{
    auto mountRes = mount(path.c_str(), path.c_str(), "", MS_BIND, nullptr);
    if (mountRes != 0) {
        log_error() << "Could not bind mount " << path << " to itself";
        return ReturnCode::FAILURE;
    }

    mountRes = mount(path.c_str(), path.c_str(), "", MS_UNBINDABLE, nullptr);
    if (mountRes != 0) {
        log_error() << "Could not make " << path << " unbindable";
        return ReturnCode::FAILURE;
    }

    mountRes = mount(path.c_str(), path.c_str(), "", MS_SHARED, nullptr);
    if (mountRes != 0) {
        log_error() << "Could not make " << path << " shared";
        return ReturnCode::FAILURE;
    }

    m_cleanupHandlers.push_back(new MountCleanUpHandler(path));
    log_debug() << "Created shared mount point at " << path;

    return ReturnCode::SUCCESS;
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

ReturnCode FileToolkitWithUndo::writeToFile(const std::string &path, const std::string &content)
{
    auto ret = softwarecontainer::writeToFile(path, content);
    if (isError(ret)) {
        return ret;
    }

    if (!pathInList(path)) {
        m_cleanupHandlers.push_back(new FileCleanUpHandler(path));
    }
    log_debug() << "Successfully wrote to " << path;
    return ReturnCode::SUCCESS;
}

void FileToolkitWithUndo::markFileForDeletion(const std::string &path)
{
    if (!pathInList(path)) {
        m_cleanupHandlers.push_back(new FileCleanUpHandler(path));
    }
}

ReturnCode FileToolkitWithUndo::createSymLink(
        const std::string &source,
        const std::string &destination)
{
    log_debug() << "creating symlink " << source << " pointing to " << destination;

    createDirectory(parentPath(source));

    if (symlink(destination.c_str(), source.c_str()) == 0) {
        if (!pathInList(source)) {
            m_cleanupHandlers.push_back(new FileCleanUpHandler(source));
        }
        log_debug() << "Successfully created symlink from " << source << " to " << destination;
    } else {
        log_error() << "Error creating symlink " << destination
                    << " pointing to " << source << ". Error: "
                    << strerror(errno);
        return ReturnCode::FAILURE;
    }
    return ReturnCode::SUCCESS;
}

} // namespace softwarecontainer
