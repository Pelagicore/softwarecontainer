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

#include "softwarecontainer-log.h"

#include <map>

#include <glibmm.h>
#include <sys/wait.h>

namespace softwarecontainer {

typedef int32_t ContainerID;
typedef std::map<std::string, std::string> EnvironmentVariables;

static constexpr pid_t INVALID_PID = -1;
static constexpr int INVALID_FD = -1;
static constexpr uid_t ROOT_UID = 0;


/**
 * @brief waitForProcessTermination Waits for a process to terminate and then returns the status
 * of the process terminating.
 * @param pid the process id to wait for.
 * @return Process termination status.
 */
inline int waitForProcessTermination(pid_t pid)
{
    int status = 0;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}

/**
 * @brief isDirectory Check if path is a directory
 * @param path Path to check
 * @return true/false
 */
bool isDirectory(const std::string &path);

/**
 * @brief isDirectoryEmpty Check if path is empty
 * @param path The path to check
 * @return false if the directory is not empty
 * @return false if the directory does not exist
 * @return true if the directory is empty
 */
bool isDirectoryEmpty(const std::string &path);

/**
 * @brief isFile Check if path is a file
 * @param path Path to check
 * @return true/false
 */
bool isFile(const std::string &path);

/**
 * @brief isPipe Check if path is a pipe
 * @param path Path to check
 * @return true/false
 */
bool isPipe(const std::string &path);

/**
 * @brief isSocket Check if path is a socket
 * @param path Path to check
 * @return true/false
 */
bool isSocket(const std::string &path);

/**
 * @brief existsInFileSystem Check if path exists
 * @param path Path to check
 * @return true/false
 */
bool existsInFileSystem(const std::string &path);

std::string parentPath(const std::string &path);
std::string baseName(const std::string &path);

bool touch(const std::string &path);
bool writeToFile(const std::string &path, const std::string &content);
bool readFromFile(const std::string &path, std::string &content);
bool parseInt(const char *args, int *result);

/*
 * This builds a full path from parts, including the appropriate separator.
 *
 * Example: buildPath("usr", "local", "bin) => usr/local/bin
 *
 * @returns a string representing the combined path
 */
std::string buildPath(const std::string &arg1, const std::string &arg2);
std::string buildPath(const std::string &arg1, const std::string &arg2, const std::string &arg3);

} // namespace softwarecontainer;
