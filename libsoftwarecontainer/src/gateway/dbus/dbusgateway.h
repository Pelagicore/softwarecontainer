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

#include "gateway.h"
#include "dbusgatewayinstance.h"

class DBusGateway : public Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("DBUS", "DBus gateway");

public:

    static constexpr const char *ID = "dbus";
    DBusGateway(const std::string &gatewayDir, const std::string &name);
    virtual ~DBusGateway();

    /**
     * @brief Sets config for both dbus session instances
     */
    virtual ReturnCode setConfig(const std::string &config) override;

    /**
     * @brief Activates both dbus session instances
     */
    virtual ReturnCode activate() override;

    /**
     * @brief Stubbed since this class only does containment
     */
    virtual ReturnCode readConfigElement(const json_t *) override { return ReturnCode::SUCCESS; }

    /**
     * @brief Stubbed since this class only does containment
     */
    virtual bool activateGateway() override { return true; }

    /**
     * @brief Stubbed since this class only does containment
     */
    virtual bool teardownGateway() override { return true; }

    virtual void setContainer(std::shared_ptr<ContainerAbstractInterface> container) override;
    virtual ReturnCode teardown() override;

    virtual bool isConfigured() override;
    virtual bool isActivated() override;

private:
    DBusGatewayInstance sessionBus;
    DBusGatewayInstance systemBus;
};
