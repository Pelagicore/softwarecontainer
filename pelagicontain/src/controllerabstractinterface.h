/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLERABSTRACTINTERFACE_H
#define CONTROLLERABSTRACTINTERFACE_H

#include <string>

/*! ControllerAbstractInterface is an abstract interface to the Controller.
 *
 *  This class is used by Pelagicontain to communicate with Controller
 *  and is intended to be an abstraction of the acutal implementation.
 */
class ControllerAbstractInterface {

public:
    virtual ~ControllerAbstractInterface() {
    };

    /*! Starts the application inside the container
     *
     * \return True if all went well, false if not
     */
    virtual bool startApp() = 0;

    /*! Stops the application running inside the container and also
     *  stops Controller.
     *
     * \return True if all went well, false if not
     */
    virtual bool shutdown() = 0;

    /*! Tells controller to set the specified environment variable
     * to the specified value. If the variable does not exist, it is
     * created.
     *
     * \param variable A string with the environment variable name
     * \param value A string with the value to set on the environment variable
     */
    virtual bool setEnvironmentVariable(const std::string &variable,
                                        const std::string &value) = 0;

    /*! Notifies the controller to issue the system call
     *  defined in the cmd argument.
     *
     * \return True if all went well, false if not
     */
    virtual bool systemCall(const std::string &cmd) = 0;

    /*! This method returns true if a call to startApp has been made previously,
     * false if not.
     *
     * \return true or false
     */
    virtual bool hasBeenStarted() const = 0;

    /*! Sets up the IPC between Pelagicontain and Controller and waits for
     *  Controller to connect. If Controller connects, we are ready to receive
     *  messages from it. Returns true if Controller connected and no other
     *  errors occured, and false if there was any error or the timeout was
     *  reached before Controller connected.
     *
     * \return true or false
     */
    virtual bool initialize() = 0;
};

#endif /* CONTROLLERABSTRACTINTERFACE_H */
