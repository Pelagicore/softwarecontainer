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
 * Gateway base class for Pelagicontain
 */
class Gateway
{
    LOG_DECLARE_CLASS_CONTEXT("GATE", "Gateway");

protected:
    static constexpr const char *ENABLED_FIELD = "enabled";

public:
    Gateway(const char* id)
    {
        m_id = id;
    };

    virtual ~Gateway()
    {
    };

    /**
     * Returns the ID of the gateway
     */
    virtual const char* id() {
        return m_id;
    }

    /*! Configure this gateway according to the supplied JSON configuration
     * string
     *
     * \param config JSON string containing gatway-specific JSON configuration
     * \returns true if \p config was sucecssfully parsed
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

    ReturnCode createSymLinkInContainer(const std::string &source, const std::string &destination);

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

    /*! Notifies the controller to issue the system call
     *  defined in the cmd argument.
     *
     * \return True if all went well, false if not
     */
    ReturnCode systemCall(const std::string &cmd)
    {
        return getContainer().systemCall(cmd);
    }

    pid_t makePopenCall(const std::string &command, int *infp, int *outfp);

    bool makePcloseCall(pid_t pid, int infp, int outfp);

protected:
    Container *m_container = nullptr;
    const char* m_id = nullptr;
};

#endif /* GATEWAY_H */
