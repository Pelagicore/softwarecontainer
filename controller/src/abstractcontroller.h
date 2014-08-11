/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef ABSTRACTCONTROLLER_H
#define ABSTRACTCONTROLLER_H

#include <string>
#include <unistd.h>

/*! AbstractController is an abstract interface to the Controller class
 *
 *  It's used by the IPC implementation to call methods on an implementation
 *  of this interface.
 */
class AbstractController {

public:
    virtual ~AbstractController() {
    };

    /*! Starts the application inside the container
     *
     * \return Pid of the application process
     */
    virtual pid_t runApp() = 0;

    /*! Stops the application running inside the container, and also stops
     * Controller
     */
    virtual void killApp() = 0;

    /*! Set the specified environment variable to the specified value
     *
     * \param variable A string with the environment variable name
     * \param value A string with the value to set on the environment variable
     */
    virtual void setEnvironmentVariable(const std::string &variable,
                                        const std::string &value) = 0;

    /*! Invoke the string passed as a system call inside the container
     *
     * \param command A string with the command to execute inside the container
     */
    virtual void systemCall(const std::string &command) = 0;
};

#endif //ABSTRACTCONTROLLER_H
