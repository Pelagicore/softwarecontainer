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

#include "createdir.h"
#include "softwarecontainererror.h"

#include <sys/stat.h>

namespace softwarecontainer {

CreateDir::~CreateDir()
{
    bool success = true;

    while (!m_rollbackCleaners.empty()) {
        auto cuHandler = m_rollbackCleaners.back();
        m_rollbackCleaners.pop_back();

        if (!cuHandler.clean()) {
            success = false;
        }
    }

    if(!success) {
        log_error() << "One or more cleanup handlers returned error status, please check the log";
    }
}


bool CreateDir::createParentDirectory(const std::string path)
{
    log_debug() << "Creating parent directories for " << path;
    std::string parent = parentPath(path);

    if (!createDirectory(parent)) {
        log_error() << "Could not create directory " << parent;
        return false;
    }

    return true;
}

bool CreateDir::createDirectory(const std::string path)
{
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

    if (!pathInList(path)){
        m_rollbackCleaners.emplace_back(path);
    }
    return true;
}

std::string CreateDir::createTempDirectoryFromTemplate(std::string templ)
{
    char *dir = const_cast<char*>(templ.c_str());
    dir = mkdtemp(dir);
    if (dir == nullptr) {
        std::string message = "Failed to create directory from template: " + std::string(strerror(errno));
        log_warning() << message;
        throw SoftwareContainerError(message);
    }

    m_rollbackCleaners.emplace_back(std::string(dir));
    return std::string(dir);
}

bool CreateDir::pathInList(const std::string path)
{
    for (auto &element : m_rollbackCleaners) {
        if (element.queryName() == path) {
            return true;
        }
    }
    return false;
}

}
