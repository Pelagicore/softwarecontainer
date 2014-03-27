/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef SYSTEMCALLINTERFACE_H
#define SYSTEMCALLINTERFACE_H

#include <string>
#include "systemcallabstractinterface.h"

/*! SystemCallInterface provides an interface to the host system.
 *
 *  This class is used by Pelagicontain to communicate with the host system
 *  and is intended to be an abstraction of the acutal implementation.
 */
class SystemCallInterface :
    public SystemCallAbstractInterface
{
public:
    SystemCallInterface();
    ~SystemCallInterface();

    /*! Implements abstract method makeCall on SystemCallAbstractInterface
     *
     * \param cmd The command sent to system().
     * \return True if the command was successfully executed.
     */
    virtual bool makeCall(const std::string &cmd);

    /*! Implements abstract method makeCall on SystemCallAbstractInterface
     *
     * \param cmd The command sent to system().
     * \param exitCode Stores the exit code returned by the call to system()
     * \return True if the command was successfully executed.
     */
    virtual bool makeCall(const std::string &cmd, int &exitCode);
};

#endif /* SYSTEMCALLINTERFACE_H */
