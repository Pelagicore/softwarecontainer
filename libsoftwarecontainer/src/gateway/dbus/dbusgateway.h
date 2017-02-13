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

#include "gateway/gateway.h"
#include "dbusgatewayinstance.h"

namespace softwarecontainer {

class DBusGateway : public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("DBUS", "DBus gateway");

public:

    static constexpr const char *ID = "dbus";
    DBusGateway(const std::string &gatewayDir,
                std::shared_ptr<ContainerAbstractInterface> container);
    virtual ~DBusGateway();

    /**
     * @brief Sets config for both dbus session instances
     */
    virtual bool setConfig(const json_t *config) override;

    /**
     * @brief Activates both dbus session instances
     */
    virtual bool activate() override;

    /**
     * @brief Stubbed since this class only does containment
     */
    virtual bool readConfigElement(const json_t *) override { return true; }

    /**
     * @brief Stubbed since this class only does containment
     */
    virtual bool activateGateway() override { return true; }

    /**
     * @brief Stubbed since this class only does containment
     */
    virtual bool teardownGateway() override { return true; }

    virtual bool teardown() override;

    virtual bool isConfigured() override;
    virtual bool isActivated() override;

private:
    DBusGatewayInstance sessionBus;
    DBusGatewayInstance systemBus;
};

} // namespace softwarecontainer
