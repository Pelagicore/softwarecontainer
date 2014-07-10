/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLERINTERFACE_H
#define CONTROLLERINTERFACE_H

#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "controllerabstractinterface.h"
#include "log.h"

/*! ControllerInterface is an interface to Controller.
 *
 *  This class is used by Pelagicontain to communicate with Controller
 *  and is intended to hide the details of the communication mechanism
 *  implementation.
 */
class ControllerInterface:
    public ControllerAbstractInterface
{
    LOG_DECLARE_CLASS_CONTEXT("CTLI", "Controller interface");

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

    /*! Implements ControllerAbstractInterface::hasBeenStarted
     */
    virtual bool hasBeenStarted() const;

    /*! Implements ControllerAbstractInterface::initialize
     */
    virtual bool initialize();

private:
    /*! Waits for Controller to connect until timeout is reached.
     *  If Controller connects before the timeout and if there were no errors,
     *  this method returns true, and false otherwise.
     *
     * \return true or false
     */
    bool isControllerConnected();

    /*! Checks if there are any messages from Controller.
     *  This method is called by a timeout signal handler until it returns false
     *  which it does only on error.
     *
     * \return true or false
     */
    bool checkForMessage();

    /*! Helper to check if the IPC is ready for sending a message to Controller.
     *
     * \return true or false
     */
    bool canSend();

    bool m_connected;
    std::string m_socketPath;
    bool m_running;
    int m_listenSocket;
    int m_connectionSocket;
    int m_highestFd;
    //TODO: This should be set through some global config
    int m_timeout;
    struct sockaddr_un m_remote;
    struct timeval m_selectTimeout;
};

#endif /* CONTROLLERINTERFACE_H */
