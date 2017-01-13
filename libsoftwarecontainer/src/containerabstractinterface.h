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

#include "softwarecontainer-common.h"
#include "executable.h"

namespace softwarecontainer {

class ContainerAbstractInterface : public Executable
{

public:

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
     * @brief Tries to bind mount a path from host to container
     *
     * Any missing parent paths will be created.
     *
     * @param pathInHost The path on the host that shall be bind mounted into the container
     * @param pathInContainer Where to mount the path in the container.
     * @param readonly Sets if the mount should be read only or read write
     *
     * @return SUCCESS if everything worked as expected, FAILURE otherwise
     */
    virtual ReturnCode bindMountInContainer(const std::string &pathInHost,
                                            const std::string &pathInContainer,
                                            bool readOnly = true) = 0;

    virtual ReturnCode setEnvironmentVariable(const std::string &variable, const std::string &value) = 0;
    virtual ReturnCode setCgroupItem(std::string subsys, std::string value) = 0;
};

} // namespace softwarecontainer
