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

#include "gateway/dbus/dbusgateway.h"
#include "gateway/gatewayparsererror.h"

namespace softwarecontainer {

DBusGateway::DBusGateway(const std::string &gatewayDir, const std::string &name):
    Gateway(ID),
    sessionBus(DBusGatewayInstance::ProxyType::SessionProxy, gatewayDir, name),
    systemBus(DBusGatewayInstance::ProxyType::SystemProxy, gatewayDir, name)
{
}

DBusGateway::~DBusGateway()
{
}

ReturnCode DBusGateway::setConfig(const std::string &config)
{
    try {
        log_debug() << "Setting config for dbus session bus";
        ReturnCode sessionBusParseResult = sessionBus.setConfig(config);

        log_debug() << "Setting config for dbus system bus";
        ReturnCode systemBusParseResult = systemBus.setConfig(config);

        if (isError(sessionBusParseResult) && isError(systemBusParseResult)) {
            log_error() << "Neither session nor system bus could use the given config";
            return ReturnCode::FAILURE;
        }
    } catch (GatewayParserError &err) {
        log_error() << "Bad config in one of the gateway configurations";
        return ReturnCode::FAILURE;
    }

     return ReturnCode::SUCCESS;
}

ReturnCode DBusGateway::activate()
{
     ReturnCode sessionBusActivationResult = ReturnCode::FAILURE;
     if (sessionBus.isConfigured()) {
         log_debug() << "Activating dbus session bus";
         sessionBusActivationResult = sessionBus.activate();
     }

     ReturnCode systemBusActivationResult = ReturnCode::FAILURE;
     if (systemBus.isConfigured()) {
         log_debug() << "Activating dbus system bus";
         sessionBusActivationResult = systemBus.activate();
     }

     if (isError(sessionBusActivationResult) && isError(systemBusActivationResult)) {
         log_error() << "Neither dbus session bus nor dbus system bus could be activated";
         return ReturnCode::FAILURE;
     }
     return ReturnCode::SUCCESS;
}

ReturnCode DBusGateway::teardown()
{
     ReturnCode sessionBusTeardownResult;
     if (sessionBus.isActivated()) {
         log_debug() << "Tearing down dbus session bus";
         sessionBusTeardownResult = sessionBus.teardown();
     } else {
         sessionBusTeardownResult = ReturnCode::FAILURE;
     }

     ReturnCode systemBusTeardownResult ;
     if (systemBus.isActivated()) {
         log_debug() << "Tearing down dbus system bus";
         systemBusTeardownResult = systemBus.teardown();
     } else {
         systemBusTeardownResult = ReturnCode::FAILURE;
     }

     if (isError(sessionBusTeardownResult) && isError(systemBusTeardownResult)) {
         log_error() << "Neither dbus session bus nor dbus system bus could be tore down";
         return ReturnCode::FAILURE;
     }

     return ReturnCode::SUCCESS;
}

void DBusGateway::setContainer(std::shared_ptr<ContainerAbstractInterface> container)
{
    sessionBus.setContainer(container);
    systemBus.setContainer(container);
}

bool DBusGateway::isConfigured()
{
    return sessionBus.isConfigured() || systemBus.isConfigured();
}

bool DBusGateway::isActivated()
{
    return sessionBus.isActivated() || systemBus.isActivated();
}

} // namespace softwarecontainer
