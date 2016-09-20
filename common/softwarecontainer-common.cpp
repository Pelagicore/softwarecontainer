
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


#include <string.h>
#include <string>
#include <fcntl.h>
#include <fstream>
#include "softwarecontainer-common.h"

#include <string>
#include <iostream>
#include <sstream>

namespace softwarecontainer {

LOG_DECLARE_DEFAULT_CONTEXT(defaultLogContext, "MAIN", "Main context");

struct stat getStat(const std::string &path)
{
    struct stat st = {};
    if (stat(path.c_str(), &st) == 0) {
        return st;
    }
    return st;
}

bool isDirectory(const std::string &path)
{
    return S_ISDIR(getStat(path).st_mode);
}

bool isFile(const std::string &path)
{
    return S_ISREG(getStat(path).st_mode);
}

bool isPipe(const std::string &path)
{
    return S_ISFIFO(getStat(path).st_mode);
}

bool isSocket(const std::string &path)
{
    return S_ISSOCK(getStat(path).st_mode);
}

bool existsInFileSystem(const std::string &path)
{
    return (getStat(path).st_mode != 0);
}

std::string parentPath(const std::string &path_)
{
    static constexpr const char *separator = "/";
    static constexpr const char separator_char = '/';

    auto path = path_;

    // Remove trailing backslashes
    while ((path.size() > 0) && (path[path.size() - 1] == separator_char)) {
        path.resize(path.size() - 1);
    }

    auto pos = path.rfind(separator);
    if (pos == std::string::npos) {
        pos = strlen(separator);
    }
    std::string parentPath = path.substr(0, pos - strlen(separator) + 1);
    return parentPath;
}

ReturnCode touch(const std::string &path)
{
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_NOCTTY | O_NONBLOCK | O_LARGEFILE, 0666);
    if (fd != -1) {
        close(fd);
        return ReturnCode::SUCCESS;
    } else {
        return ReturnCode::FAILURE;
    }
}

ReturnCode writeToFile(const std::string &path, const std::string &content)
{
    ReturnCode ret = ReturnCode::SUCCESS;
    log_verbose() << "writing to " << path << " : " << content;
    std::ofstream out(path);
    if (out.is_open()) {
        out << content;
        if (!out.good()) {
            ret = ReturnCode::FAILURE;
        }
        out.close();
    } else {
        ret = ReturnCode::FAILURE;
    }
    return ret;
}

ReturnCode readFromFile(const std::string &path, std::string &content)
{
    ReturnCode ret = ReturnCode::SUCCESS;
    std::ifstream t(path);
    if (t.is_open()) {
        std::stringstream buffer;
        buffer << t.rdbuf();
        content = buffer.str();
        if (!t.good()) {
            ret = ReturnCode::FAILURE;
        }
        t.close();
    } else {
        ret = ReturnCode::FAILURE;
    }
    return ret;
}

bool parseInt(const char *arg, int *result)
{
    char *end;
    long value = strtol(arg, &end, 10);
    if (end == arg || *end != '\0' || errno == ERANGE) {
        return false;
    }

    *result = value;
    return true;
}
FileToolkitWithUndo::~FileToolkitWithUndo()
{
    bool success = true;
    // Clean up all created directories, files, and mount points
    for (auto it = m_cleanupHandlers.rbegin(); it != m_cleanupHandlers.rend(); ++it) {
        // Cleaning functions will do their own error/warning output
        if(isError((*it)->clean())) {
            success = false;
        }
        delete *it;
    }

    if(!success) {
        log_error() << "One or more cleanup handlers returned error status, please check the log";
    }
}

ReturnCode FileToolkitWithUndo::createParentDirectory(const std::string &path)
{
    log_debug() << "Creating parent directories for " << path;
    std::string parent = parentPath(path);
    if (!isDirectory(parent) && parent != "") {
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

    m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(path));
    log_debug() << "Created directory " << path;

    return ReturnCode::SUCCESS;
}

std::string FileToolkitWithUndo::tempDir(std::string templ)
{
    char *dir = const_cast<char*>(templ.c_str());
    dir = mkdtemp(dir);
    if (dir == NULL) {
        log_warning() << "Failed to create buffered Directory: " << strerror(errno);
        return nullptr;
    }

    m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(templ));

    return StringBuilder() << dir;
}

ReturnCode FileToolkitWithUndo::bindMount(const std::string &src, const std::string &dst, bool readOnly, bool enableWriteBuffer)
{
    unsigned long flags = MS_BIND;
    std::string fstype;
    const void *data = nullptr;
    int mountRes;
    log_debug() << "Bind-mounting " << src << " in " << dst << ", flags: " << flags;

    if(enableWriteBuffer) {
        std::string upperDir = tempDir("/tmp/sc-bindmount-upper-XXXXXX");
        std::string workDir = tempDir("/tmp/sc-bindmount-work-XXXXXX");
        fstype.assign("overlay");

        std::ostringstream os;
        os << "lowerdir=" << src << ",upperdir=" << upperDir << ",workdir=" << workDir;
        data = os.str().c_str();

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
        const std::string &lower
        , const std::string &upper
        , const std::string &work
        , const std::string &dst)
{
    std::string fstype = "overlay";
    unsigned long flags = MS_BIND;

    if ((createDirectory(lower) && ReturnCode::FAILURE)
        || (createDirectory(upper) && ReturnCode::FAILURE)
        || (createDirectory(work) && ReturnCode::FAILURE))
    {
        log_error() << "Failed to create lower/upper/work directory for overlayMount. lower=" <<
                       lower << ", upper=" << upper << ", work=" << work;
        return ReturnCode::FAILURE;
    }

    std::string mountoptions = StringBuilder() << "lowerdir=" << lower
                                               << ",upperdir=" << upper
                                               << ",workdir=" << work;

    int mountRes = mount("overlay", dst.c_str(), fstype.c_str(), flags, mountoptions.c_str());

    if (mountRes == 0) {
        log_verbose() << "overlayMounted folder " << lower << " in " << dst;
        m_cleanupHandlers.push_back(new MountCleanUpHandler(dst));
        m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(upper));
        m_cleanupHandlers.push_back(new DirectoryCleanUpHandler(work));
    } else {
        log_error() << "Could not mount into container: lower=" << lower
                    << " , dst=" << dst << " err=" << strerror(errno);
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

ReturnCode FileToolkitWithUndo::createSharedMountPoint(const std::string &path)
{
    // MS_MGC_VAL |
    auto mountRes = mount(path.c_str(), path.c_str(), "", MS_BIND, nullptr);
    assert(mountRes == 0);
    mountRes = mount(path.c_str(), path.c_str(), "", MS_UNBINDABLE, nullptr);
    assert(mountRes == 0);
    mountRes = mount(path.c_str(), path.c_str(), "", MS_SHARED, nullptr);
    assert(mountRes == 0);
    m_cleanupHandlers.push_back(new MountCleanUpHandler(path));
    log_debug() << "Created shared mount point at " << path;

    return ReturnCode::SUCCESS;
}

ReturnCode FileToolkitWithUndo::writeToFile(const std::string &path, const std::string &content)
{
    auto ret = softwarecontainer::writeToFile(path, content);
    if (isError(ret)) {
        return ret;
    }
    m_cleanupHandlers.push_back(new FileCleanUpHandler(path));
    log_debug() << "Successfully wrote to " << path;
    return ReturnCode::SUCCESS;
}

ReturnCode FileToolkitWithUndo::createSymLink(const std::string &source, const std::string &destination)
{
    log_debug() << "creating symlink " << source << " pointing to " << destination;

    createDirectory(parentPath(source));

    if (symlink(destination.c_str(), source.c_str()) == 0) {
        m_cleanupHandlers.push_back(new FileCleanUpHandler(source));
        log_debug() << "Successfully created symlink from " << source << " to " << destination;
    } else {
        log_error() << "Error creating symlink " << destination
                    << " pointing to " << source << ". Error: "
                    << strerror(errno);
        return ReturnCode::FAILURE;
    }
    return ReturnCode::SUCCESS;
}

MountCleanUpHandler::MountCleanUpHandler(const std::string &path)
{
    m_path = path;
}

ReturnCode MountCleanUpHandler::clean()
{
    // Lazy unmount. Should be the equivalent of the "umount -l" command.
    if (umount2(m_path.c_str(), MNT_DETACH) == 0) {
        log_debug() << "Unmounted " << m_path;
        return ReturnCode::SUCCESS;
    } else {
        log_warn() << "Can't unmount " << m_path << " . Error :" << strerror(errno) << ". Trying to force umount";
        if (umount2(m_path.c_str(), MNT_FORCE) != 0) {
            log_warn() << "Can't force unmount " << m_path << " . Error :" << strerror(errno);
            return ReturnCode::FAILURE;
        }
        log_debug() << "Managed to force unmount " << m_path;
        return ReturnCode::SUCCESS;
    }
}

FileCleanUpHandler::FileCleanUpHandler(const std::string &path)
{
    m_path = path;
}

ReturnCode FileCleanUpHandler::clean()
{
    if (unlink(m_path.c_str()) == 0) {
        log_debug() << "Unlinked " << m_path;
        return ReturnCode::SUCCESS;
    } else {
        log_error() << "Can't delete " << m_path << " . Error :" << strerror(errno);
        return ReturnCode::FAILURE;
    }
}

void SignalConnectionsHandler::addConnection(sigc::connection &connection) {
    m_connections.push_back(connection);
}

SignalConnectionsHandler::~SignalConnectionsHandler()
{
    for (auto &connection : m_connections) {
        connection.disconnect();
    }
}

bool operator&& (ReturnCode lhs, ReturnCode rhs) {
    return lhs == rhs;
}

}



