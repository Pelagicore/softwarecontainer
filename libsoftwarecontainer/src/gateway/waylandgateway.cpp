
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


#include <string>
#include <unistd.h>
#include "waylandgateway.h"

// TODO: no clue why the following lines are needed
constexpr const char *Gateway::ENABLED_FIELD;
constexpr const char *WaylandGateway::SOCKET_FILE_NAME;
constexpr const char *WaylandGateway::WAYLAND_RUNTIME_DIR_VARIABLE_NAME;

WaylandGateway::WaylandGateway() :
    Gateway(ID)
{
}

WaylandGateway::~WaylandGateway()
{
}

ReturnCode WaylandGateway::readConfigElement(const json_t *element)
{
    json_t *enabled_value = json_object_get(element, ENABLED_FIELD);
    if (!enabled_value) {
        log_error() << "Key " << ENABLED_FIELD << " missing in json configuration";
        return ReturnCode::FAILURE;
    }

    if (json_is_boolean(enabled_value)) {
        m_enabled = json_is_true(enabled_value);
    } else {
        log_error() << "Value for " << ENABLED_FIELD << " key is not a boolean value";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

bool WaylandGateway::activateGateway()
{
    if (m_enabled) {
        const char *dir = getenv(WAYLAND_RUNTIME_DIR_VARIABLE_NAME);
        if (dir != nullptr) {
            log_info() << "enabling Wayland gateway. Socket dir:" << dir;
            std::string d = logging::StringBuilder() << dir << "/" << SOCKET_FILE_NAME;
            std::string path;
            ReturnCode result = getContainer()->bindMountFileInContainer(d, SOCKET_FILE_NAME, path, false);
            if (isError(result)) {
                log_error() << "Could not bind mount the wayland socket into the container";
                return false;
            }

            setEnvironmentVariable(WAYLAND_RUNTIME_DIR_VARIABLE_NAME, parentPath(path));
        } else {
            log_error() << "Should enable wayland gateway, but " << WAYLAND_RUNTIME_DIR_VARIABLE_NAME << " is not defined";
            return false;
        }
    } else {
        log_info() << "Wayland gateway disabled";
    }
    return true;
}

bool WaylandGateway::teardownGateway()
{
    return true;
}
