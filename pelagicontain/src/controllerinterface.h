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
class ControllerInterface :
    public ControllerAbstractInterface
{
public:
    ControllerInterface(const std::string &containerRoot);

    ~ControllerInterface();

    /*! Implements ControllerAbstractInterface::startApp
    */
    virtual bool startApp();

    /*! Implements ControllerAbstractInterface::shutdown
    */
    virtual bool shutdown();

private:
    bool fifoExist();
    void openFifo();

    int m_fifo;
    std::string m_fifoPath;
};

#endif /* CONTROLLERINTERFACE_H */
