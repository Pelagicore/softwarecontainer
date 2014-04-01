/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLERINTERFACE_H
#define CONTROLLERINTERFACE_H

#include <string>

#include "controllerabstractinterface.h"

/*! ControllerInterface is an interface to Controller.
 *
 *  This class is used by Pelagicontain to communicate with Controller
 *  and is intended to hide the details of the communication mechanism
 *  implementation.
 */
class ControllerInterface:
    public ControllerAbstractInterface
{
public:
    ControllerInterface(const std::string &gatewayDir);

    ~ControllerInterface();

    /*! Implements ControllerAbstractInterface::startApp
     */
    virtual bool startApp();

    /*! Implements ControllerAbstractInterface::shutdown
     */
    virtual bool shutdown();

    /*! Implements ControllerAbstractInterface::setEnvironmentVariable
     */
    virtual bool setEnvironmentVariable(const std::string &variable,
                                        const std::string &value);

    /*! Implements ControllerAbstractInterface::systemCall
     */
    virtual bool systemCall(const std::string &cmd);

private:
    bool openFifo();

    int m_fifo;
    std::string m_fifoPath;
};

#endif /* CONTROLLERINTERFACE_H */
