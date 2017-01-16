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

#include "workspace.h"

namespace softwarecontainer {

Workspace::Workspace()
{
    // Make sure path ends in '/' since it might not always be checked
    if (m_containerRootDir.back() != '/') {
        m_containerRootDir += "/";
    }

    if (isError(checkWorkspace())) {
        throw SoftwareContainerError("Failed to initialize Workspace");
    }
}

Workspace::~Workspace()
{
}

ReturnCode Workspace::checkWorkspace()
{
    if (!isDirectory(m_containerRootDir)) {
        log_debug() << "Container root "
                    << m_containerRootDir
                    << " does not exist, trying to create";

        if(isError(createDirectory(m_containerRootDir))) {
            log_debug() << "Failed to create container root directory";
            return ReturnCode::FAILURE;
        }
    }

#ifdef ENABLE_NETWORKGATEWAY
    // TODO: Have a way to check for the bridge using C/C++ instead of a
    // shell script. Libbridge and/or netfilter?
    std::string cmdLine = std::string(INSTALL_BINDIR) + "/setup_softwarecontainer.sh";
    log_debug() << "Creating workspace : " << cmdLine;
    int returnCode;
    try {
        Glib::spawn_sync("", Glib::shell_parse_argv(cmdLine), Glib::SPAWN_DEFAULT,
                         sigc::slot<void>(), nullptr,
                         nullptr, &returnCode);

    } catch (Glib::SpawnError e) {
        log_error() << "Failed to spawn " << cmdLine << ": code " << e.code()
                    << " msg: " << e.what();

        return ReturnCode::FAILURE;
    }

    if (returnCode != 0) {
        log_error() << "Return code of " << cmdLine << " is non-zero";
        return ReturnCode::FAILURE;
    }
#endif

    return ReturnCode::SUCCESS;
}

} // namespace softwarecontainer
