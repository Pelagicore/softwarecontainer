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

#include <string>
#include <exception>

#include "containerabstractinterface.h"
#include "jsonparser.h"
#include "softwarecontainererror.h"

namespace softwarecontainer {

class GatewayError : public SoftwareContainerError
{
public:
    GatewayError():
        SoftwareContainerError("Gateway error.")
    {
    }

    GatewayError(const std::string &message):
        SoftwareContainerError(message)
    {
    }
};

/**
 * @brief Gateway base class for SoftwareContainer
 *
 * Gateways can be in one of three states:
 * * Created - the gateway object exists
 * * Configured - the gateway has successfully recieved and processed some
 *                configuration data, ready for activation
 * * Activated - the gateway is active and running.
 *
 * The complete gateway config is passed as a JSON array and all
 * gateways then provide their specific parsing of the items in the array.
 *
 * Gateways can specify if they are 'dynamic' or not when initializing this
 * base class. A dynamic gateway supports multiple calls to setConfig and
 * activate.
 */
class Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("GATE", "Gateway");

public:
    enum class GatewayState : unsigned int {
        CREATED = 0,
        CONFIGURED = 1,
        ACTIVATED = 2,
    };

    /**
     * @brief Constructor for inheriting classes to initilize
     *
     * Gateways that supports quick-launch should flag for this when
     * initializing this base class.
     *
     * @param id Specific gateway ID used to match with configurations.
     *
     * @param container An interface to the Container that this gateway is
     *                  part of.
     *
     * @param isDynamic Set to 'true' to indicate the inheriting
     *                  class supports re-configuration and re-activation
     */
    Gateway(const std::string &id,
            std::shared_ptr<ContainerAbstractInterface> container,
            bool isDynamic = false);

    virtual ~Gateway() {}

    /**
     * @brief Returns the ID of the gateway
     *
     * @return Returns the ID of the gateway as a string
     */
    virtual std::string id() const;

    /**
     * @brief Configure this gateway according to the supplied JSON configuration
     *        string
     *
     * @param config JSON string containing gateway-specific JSON configuration
     *
     * @returns true if \p config was successfully parsed,
     *          false otherwise
     *
     * @throws GatewayError If called on an already activated gateway.
     */
    virtual bool setConfig(const json_t *config);

    /**
     * @brief Applies any configuration set by setConfig()
     *
     * @returns true upon successful application of configuration,
     *          false otherwise
     *
     * @throws GatewayError If called on an already activated gateway, or if the
     *                      gateway has not been previously configured, or if there
     *                      is not container instance set.
     */
    virtual bool activate();

    /**
     * @brief Restore system to the state prior to launching of gateway. Any cleanup
     *  code (removal of files, virtual interfaces, etc) should be placed here.
     *
     * @returns true upon successful clean-up,
     *          false otherwise
     *
     * @throws GatewayError If called on a non activated gateway.
     */
    virtual bool teardown();

    /**
     * @brief Is the gateway configured or not?
     */
    virtual bool isConfigured();

    /**
     * @brief Is the gateway activated or not?
     *
     * Dynamic gateways will return true if they have been activated at least
     * once. Non-dynamic gateways will return true if they are in state ACTIVATED
     */
    virtual bool isActivated();

protected:
    /**
     * @brief Gateway specific parsing of config elements
     *
     * All gateways implement this method in order to provide gateway
     * specific parsing of the configuration content.
     *
     * @param element A JSON configuration item.
     * @returns false if an error was encountered while parsing, true otherwise.
     */
    virtual bool readConfigElement(const json_t *element) = 0;

    /**
     * @brief Get a handle to the associated container
     *
     * @throws GatewayError If called before setContainer() has been called.
     */
    std::shared_ptr<ContainerAbstractInterface> getContainer();

    /**
     * @brief Set an environment variable in the associated container
     */
    bool setEnvironmentVariable(const std::string &variable, const std::string &value);

    virtual bool activateGateway() = 0;
    virtual bool teardownGateway() = 0;

    /*
     * Dynamic gateways must set this to true the first time they are activated.
     * This is used by this class to keep track of this, but it should also be expected
     * that inheriting gateway implementations to use this member to keep
     * track of this state as well.
     */
    bool m_activatedOnce;

private:
    std::string m_id;
    std::shared_ptr<ContainerAbstractInterface> m_container;

    // Dynamic gateways must set this on initialization to enable dynamic behavior
    bool m_isDynamic;

    GatewayState m_state;

};

} // namespace softwarecontainer
