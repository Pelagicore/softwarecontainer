/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef GATEWAY_H
#define GATEWAY_H

#include <string>
#include "container.h"

#include <wait.h>

/*! Gateway base class
 *
 * Gateway base class for SoftwareContainer
 */
class Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("GATE", "Gateway");

protected:
    static constexpr const char *ENABLED_FIELD = "enabled";

    static constexpr const char *XDG_RUNTIME_DIR_VARIABLE_NAME = "XDG_RUNTIME_DIR";

public:
    Gateway(const char *id)
    {
        m_id = id;
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
    virtual ReturnCode readConfigElement(const JSonElement &element)
    {
        return ReturnCode::SUCCESS;
    }

    /*! Applies any configuration set by setConfig()
     *
     * \returns true upon successful application of configuration
     *          false otherwise
     */
    virtual bool activate() = 0;

    /*! Restore system to the state prior to launching of gateway. Any cleanup
     * code (removal of files, virtual interfaces, etc) should be placed here.
     *
     * \returns true upon successful clean-up, false otherwise
     */
    virtual bool teardown()
    {
        return true;
    }

    bool hasContainer()
    {
        return m_container != nullptr;
    }

    Container &getContainer()
    {
        return *m_container;
    }

    void setContainer(Container &container)
    {
        m_container = &container;
    }

    ReturnCode setEnvironmentVariable(const std::string &variable, const std::string &value)
    {
        return getContainer().setEnvironmentVariable(variable, value);
    }

    /*! Execute the given command in the container
     */
    ReturnCode executeInContainer(const std::string &cmd)
    {
        return getContainer().executeInContainer(cmd);
    }

protected:
    Container *m_container = nullptr;
    const char *m_id = nullptr;
};

#endif /* GATEWAY_H */
