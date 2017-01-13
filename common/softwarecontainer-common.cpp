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

#include "softwarecontainer-common.h"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

namespace softwarecontainer {
    LOG_DECLARE_DEFAULT_CONTEXT(defaultLogContext, "MAIN", "Main context");

struct stat getStat(const std::string &path)
{
    struct stat st;
    memset(&st, 0, sizeof(st));

    if (stat(path.c_str(), &st) == 0) {
        return st;
    }
    return st;
}

bool isDirectory(const std::string &path)
{
    return S_ISDIR(getStat(path).st_mode);
}

bool isDirectoryEmpty(const std::string &path) {
    int n = 0;
    constexpr int EMPTY_DIR_SIZE=2;
    struct dirent *d;
    DIR *dir = opendir(path.c_str());
    if (dir == NULL) { //Not a directory or doesn't exist
        return true;
    }
    while ((d = readdir(dir)) != NULL) {
        ++n;
        if(n > EMPTY_DIR_SIZE) {
            break;
        }
    }
    closedir(dir);
    if (n <= EMPTY_DIR_SIZE) { //Directory Empty
        return true;
    } else {
        return false;
    }
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

} // namespace softwarecontainer

