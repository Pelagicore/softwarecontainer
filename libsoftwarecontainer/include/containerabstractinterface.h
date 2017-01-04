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


#pragma once

#include "softwarecontainer-common.h"

namespace softwarecontainer {

class ContainerAbstractInterface {

public:
    // A function to be executed in the container
    typedef std::function<int ()> ContainerFunction;

    virtual ~ContainerAbstractInterface() {};
    virtual const char *id() const = 0;
    virtual const std::string &rootFS() const = 0;
    virtual std::string toString() = 0;

    virtual ReturnCode initialize() = 0;
    virtual ReturnCode create() = 0;
    virtual ReturnCode start(pid_t *pid) = 0;
    virtual ReturnCode stop() = 0;

    virtual ReturnCode shutdown() = 0;
    virtual ReturnCode shutdown(unsigned int timeout) = 0;

    virtual ReturnCode suspend() = 0;
    virtual ReturnCode resume() = 0;

    virtual ReturnCode destroy() = 0;
    virtual ReturnCode destroy(unsigned int timeout) = 0;

    virtual ReturnCode mountDevice(const std::string &pathInHost) = 0;
    virtual ReturnCode createSymLink(const std::string &source, const std::string &destination) = 0;

    /**
     * @brief Tries to bind mount a file in the container
     *
     * This will not create parent directories if they are not already present.
     *
     * @param pathOnHost The path to the file that shall be bind mounted on the host system.
     * @param pathInContainer Where to mount the file in the container.
     * @param readonly Sets if the mount should be read only or read write
     *
     * @return SUCCESS if everything worked as expected, FAILURE otherwise.
     */
    virtual ReturnCode bindMountFileInContainer(const std::string &pathOnHost, const std::string &pathInContainer, bool readonly = true) = 0;

    /**
     * @brief Tries to bind mount a directory in the container
     *
     * Will create missing parent directories.
     *
     * @param pathOnHost The path to the directory that shall be bind mounted on the host system.
     * @param pathInContainer Where to mount the file in the container.
     * @param readonly Sets if the mount should be read only or read write
     *
     * @return SUCCESS if everything worked as expected, FAILURE otherwise.
     */
    virtual ReturnCode bindMountFolderInContainer(const std::string &pathOnHost, const std::string &pathInContainer, bool readonly = true) = 0;

    virtual ReturnCode setEnvironmentVariable(const std::string &variable, const std::string &value) = 0;
    virtual ReturnCode setCgroupItem(std::string subsys, std::string value) = 0;

    virtual ReturnCode attach(const std::string &commandLine, pid_t *pid,
                              const EnvironmentVariables &variables,
                              const std::string &workingDirectory = "/",
                              int stdin = -1, int stdout = 1, int stderr = 2) = 0;
    virtual ReturnCode attach(const std::string &commandLine, pid_t *pid) = 0;

    virtual ReturnCode executeInContainer(const std::string &cmdline) = 0;
    virtual ReturnCode executeInContainer(ContainerFunction function, pid_t *pid,
                                          const EnvironmentVariables &variables = EnvironmentVariables(),
                                          int stdin = -1,
                                          int stdout = -1,
                                          int stderr = 2) = 0;

};

} // namespace softwarecontainer
