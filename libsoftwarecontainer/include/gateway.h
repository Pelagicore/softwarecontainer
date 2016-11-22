
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

#include <string>

#include "containerabstractinterface.h"
#include "jsonparser.h"

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

    Gateway(const char *id)
    {
        m_id = id;
        m_state = GatewayState::CREATED;
    }

    virtual ~Gateway()
    {
    }

    /**
     * @brief
     * @return Returns the ID of the gateway
     */
    virtual const char *id()
    {
        return m_id;
    }

    /**
     * @brief Configure this gateway according to the supplied JSON configuration
     *  string
     *
     * @param config JSON string containing gateway-specific JSON configuration
     * @returns true if \p config was successfully parsed
     *          false otherwise
     */
    virtual bool setConfig(const std::string &config);

    /**
     * @brief Applies any configuration set by setConfig()
     * @returns true upon successful application of configuration, false otherwise
     */
    virtual bool activate();

    /**
     * @brief Restore system to the state prior to launching of gateway. Any cleanup
     *  code (removal of files, virtual interfaces, etc) should be placed here.
     *
     * @returns true upon successful clean-up, false otherwise
     */
    virtual bool teardown();

    /**
     * @brief Set the associated container for this gateway
     */
    void setContainer(std::shared_ptr<ContainerAbstractInterface> container);

    /**
     * @brief Is the gateway configured or not?
     */
    bool isConfigured();

    /**
     * @brief Is the gateway activated or not?
     */
    bool isActivated();

protected:
    /**
     * @brief Gateway specific parsing of config elements
     *
     * All gateways implement this method in order to provide gateway
     * specific parsing of the configuration content.
     *
     * @param element A JSON configuration item.
     * @returns ReturnCode FAILURE if an error was encountered while parsing, SUCCESS otherwise.
     */
    virtual ReturnCode readConfigElement(const json_t *element) = 0;

    /**
     * @brief Check if the gateway has an associated container
     *
     * Inheriting gateways calls this method to know if they are safe to assume they
     * can access a ContainerAbstractInterface instance previously set by SoftwareContainer
     * calling setContainer()
     *
     * @returns true if there is a ContainerAbstractInterface instance set, false if not.
     */
    virtual bool hasContainer();

    /**
     * @brief Get a handle to the associated container
     */
    std::shared_ptr<ContainerAbstractInterface> getContainer();

    /**
     * @brief Set an environment variable in the associated container
     */
    ReturnCode setEnvironmentVariable(const std::string &variable, const std::string &value);

    /**
     * @brief Execute the given command in the container
     */
    ReturnCode executeInContainer(const std::string &cmd);

    /**
     * @brief Execute a function in the container
     */
    typedef std::function<int ()> ContainerFunction;
    ReturnCode executeInContainer(ContainerFunction func);

    static constexpr const char *ENABLED_FIELD = "enabled";
    static constexpr const char *XDG_RUNTIME_DIR_VARIABLE_NAME = "XDG_RUNTIME_DIR";

    virtual bool activateGateway() = 0;
    virtual bool teardownGateway() = 0;

private:

    std::shared_ptr<ContainerAbstractInterface> m_container;
    const char *m_id = nullptr;
    GatewayState m_state = GatewayState::CREATED;

};
