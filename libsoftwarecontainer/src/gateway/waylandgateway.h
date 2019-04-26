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

#include "gateway.h"

namespace softwarecontainer {

class WaylandGateway :
    public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("WAGW", "Wayland gateway");

public:
    static constexpr const char *ID = "wayland";
    static constexpr const char *WAYLAND_RUNTIME_DIR_VARIABLE_NAME = "XDG_RUNTIME_DIR";
    static constexpr const char *WAYLAND_SOCKET_FILE_VARIABLE_NAME = "WAYLAND_DISPLAY";
    static constexpr const char *ENABLED_FIELD = "enabled";

    WaylandGateway(std::shared_ptr<ContainerAbstractInterface> container);
    ~WaylandGateway();

    bool readConfigElement(const json_t *element);
    virtual bool activateGateway();
    virtual bool teardownGateway();

private:
    bool m_enabled;
    bool m_activatedOnce;
};

} // namespace softwarecontainer
