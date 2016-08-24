
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


#ifndef GATEWAY_H
#define GATEWAY_H

#include <jansson.h>
#include "container.h"

/*! Gateway base class
 *
 * Gateway base class for SoftwareContainer
 *
 * Gateways can be in one of three states:
 * * Created - the gateway object exists
 * * Configured - the gateway has successfully recieved and processed some
 *                configuration data, ready for activation
 * * Activated - the gateway is active and running.
 *
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
     * Returns the ID of the gateway
     */
    virtual const char *id()
    {
        return m_id;
    }

    /*! Configure this gateway according to the supplied JSON configuration
     * string
     *
     * \param config JSON string containing gateway-specific JSON configuration
     * \returns true if \p config was successfully parsed
     *          false otherwise
     */
    virtual bool setConfig(const std::string &config);
    virtual ReturnCode readConfigElement(const json_t *element) = 0;

    /*! Applies any configuration set by setConfig()
     *
     * \returns true upon successful application of configuration
     *          false otherwise
     */
    virtual bool activate();

    /*! Restore system to the state prior to launching of gateway. Any cleanup
     * code (removal of files, virtual interfaces, etc) should be placed here.
     *
     * \returns true upon successful clean-up, false otherwise
     */
    virtual bool teardown();

    /*! Check if the gateway has an associated container */
    bool hasContainer();

    /*! Get a reference to the associated container */
    std::shared_ptr<ContainerAbstractInterface> getContainer();

    /*! Set the associated contaitner for this gateway */
    void setContainer(std::shared_ptr<ContainerAbstractInterface> container);

    /*! Is the gateway configured or not? */
    bool isConfigured();

    /*! Is the gateway activated or not? */
    bool isActivated();

    ReturnCode setEnvironmentVariable(const std::string &variable, const std::string &value);

    /*! Execute the given command in the container */
    ReturnCode executeInContainer(const std::string &cmd);

protected:
    static constexpr const char *ENABLED_FIELD = "enabled";
    static constexpr const char *XDG_RUNTIME_DIR_VARIABLE_NAME = "XDG_RUNTIME_DIR";

    std::shared_ptr<ContainerAbstractInterface> m_container;
    const char *m_id = nullptr;
    GatewayState m_state = GatewayState::CREATED;

    virtual bool activateGateway() = 0;
    virtual bool teardownGateway() = 0;
};

#endif /* GATEWAY_H */
