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

    /*! Human readable identifier of this gateway */
    virtual std::string id() = 0;

    /*! Configure this gateway according to the supplied JSON configuration
     * string
     *
     * \param config JSON string containing gatway-specific JSON configuration
     * \returns true if \p config was sucecssfully parsed
     *          false otherwise
     */
    virtual bool setConfig(const std::string &config) = 0;

    virtual bool activate() = 0;
    virtual bool teardown() {return true;}

protected:
    ControllerAbstractInterface *m_controllerInterface;
};

#endif /* GATEWAY_H */
