
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

    virtual ReturnCode destroy() = 0;
    virtual ReturnCode destroy(unsigned int timeout) = 0;

    virtual ReturnCode setUser(uid_t userID) = 0;

    virtual ReturnCode mountDevice(const std::string &pathInHost) = 0;
    virtual ReturnCode createSymLink(const std::string &source, const std::string &destination) = 0;
    virtual ReturnCode bindMountFileInContainer(const std::string &src, const std::string &dst, std::string &result, bool readonly = true) = 0;

    /*! Bind mount a directory from the host into the container.
     *
     * Bind mount src to dst (relative to /gateways in container), e.g. passing '/home/foo/temp' as src and
     * 'app' as dst, the resulting path inside the container will be '/gateways/app' and contain whatever is
     * in '/home/foo/temp' on the host.
     *
     * 'src' and 'dst' can be any value but there might be issues if something like '..' is used.
     *
     * If 'readonly' is set to 'true' the mount is read only.
     *
     * The resulting path is written to the 'result' out-parameter.
     */
    virtual ReturnCode bindMountFolderInContainer(const std::string &src, const std::string &dst, std::string &result, bool readonly = true) = 0;

    virtual ReturnCode setEnvironmentVariable(const std::string &variable, const std::string &value) = 0;
    virtual ReturnCode setCgroupItem(std::string subsys, std::string value) = 0;

    virtual ReturnCode attach(const std::string &commandLine, pid_t *pid, const EnvironmentVariables &variables, uid_t userID,
                              const std::string &workingDirectory = "/", int stdin = -1, int stdout = 1, int stderr = 2) = 0;
    virtual ReturnCode attach(const std::string &commandLine, pid_t *pid, uid_t userID = ROOT_UID) = 0;

    virtual ReturnCode executeInContainer(const std::string &cmdline) = 0;
    virtual ReturnCode executeInContainer(ContainerFunction function, pid_t *pid,
                                          const EnvironmentVariables &variables = EnvironmentVariables(),
                                          uid_t userID = ROOT_UID,
                                          int stdin = -1,
                                          int stdout = -1,
                                          int stderr = 2) = 0;

};
