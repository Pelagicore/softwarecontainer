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

#include <string>
#include <unistd.h>
#include "waylandgateway.h"

namespace softwarecontainer {

// These lines are needed in order to define the fields, which otherwise would
// yield linker errors.
constexpr const char *WaylandGateway::ENABLED_FIELD;
constexpr const char *WaylandGateway::SOCKET_FILE_NAME;
constexpr const char *WaylandGateway::WAYLAND_RUNTIME_DIR_VARIABLE_NAME;

WaylandGateway::WaylandGateway(std::shared_ptr<ContainerAbstractInterface> container) :
    Gateway(ID, container, true /*this GW is dynamic*/),
    m_enabled(false),
    m_activatedOnce(false)
{
}

WaylandGateway::~WaylandGateway()
{
}

bool WaylandGateway::readConfigElement(const json_t *element)
{
    bool configValue = false;

    if (!JSONParser::read(element, ENABLED_FIELD, configValue)) {
        log_error() << "Key " << ENABLED_FIELD << " missing or not bool in json configuration";
        return false;
    }

    if (!m_enabled) {
        m_enabled = configValue;
    }

    return true;
}

bool WaylandGateway::activateGateway()
{
    if (!m_enabled) {
        log_info() << "Wayland gateway disabled";
        return true;
    }

    /* If we are enabled and have been activated once already, we shouldn't do anything more
       since bind-mounting again would fail and the whitelisting principle means
       that we never need to do more than what is already done in this state, e.g.
       'enabled' is the most permissive state. */
    if (m_enabled && m_activatedOnce) {
        log_info() << "Ignoring redundant activation";
        return true;
    }

    bool hasWayland = false;
    std::string dir = Glib::getenv(WAYLAND_RUNTIME_DIR_VARIABLE_NAME, hasWayland);
    if (!hasWayland) {
        log_error() << "Should enable wayland gateway, but " << WAYLAND_RUNTIME_DIR_VARIABLE_NAME << " is not defined";
        return false;
    }

    std::shared_ptr<ContainerAbstractInterface> container = getContainer();

    log_info() << "enabling Wayland gateway. Socket dir:" << dir;
    std::string pathInHost = buildPath(dir, SOCKET_FILE_NAME);
    std::string pathInContainer = buildPath("/gateways", SOCKET_FILE_NAME);

    if (!container->bindMountInContainer(pathInHost, pathInContainer, false)) {
        log_error() << "Could not bind mount the wayland socket into the container";
        return false;
    }

    std::string socketDir = parentPath(pathInContainer);
    container->setEnvironmentVariable(WAYLAND_RUNTIME_DIR_VARIABLE_NAME, socketDir);

    m_activatedOnce = true;

    return true;
}

bool WaylandGateway::teardownGateway()
{
    return true;
}

} // namespace softwarecontainer
