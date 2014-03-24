/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef GATEWAY_H
#define GATEWAY_H

#include <string>
#include "controllerinterface.h"

/*! Gateway base class
 *
 * Gateway base class for Pelagicontain
 */
class Gateway
{
public:
    Gateway(ControllerAbstractInterface *controllerInterface):
        m_controllerInterface(controllerInterface){};
    Gateway() {};
    virtual ~Gateway() {};

    /*! Additions to the environment relevant to this gateway. The string
     * returned by this function must be usable with the 'env' util of
     * coreutils
     *
     * \returns Core-utils compatible environment
     */
    virtual std::string environment() = 0;

    /*! Used by pelagicontain to map configurations to gateways */
    virtual std::string id() = 0;

    /*! Configure this gateway according to the supplied JSON configuration
     * string
     *
     * \param config JSON string containing gatway-specific JSON configuration
     * \returns true if \p config was sucecssfully parsed
     *          false otherwise
     */
    virtual bool setConfig(const std::string &config) = 0;

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
    virtual bool teardown() {return true;}

protected:
    ControllerAbstractInterface *m_controllerInterface;
};

#endif /* GATEWAY_H */
