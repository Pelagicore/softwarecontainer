/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef SYSTEMCALLABSTRACTINTERFACE_H
#define SYSTEMCALLABSTRACTINTERFACE_H

#include <string>

/*! SystemCallAbstractInterface provides an abstract interface to the system.
 *
 *  This class is used by Pelagicontain to communicate with the host system
 *  and is intended to be an abstraction of the acutal implementation.
 */
class SystemCallAbstractInterface {

public:

    virtual ~SystemCallAbstractInterface() {};

    /*! Issues a call to system() with the command passed as argument
     *
     * \return True if the command was successfully executed.
     */
    virtual int makeCall(const std::string &cmd) = 0;

};

#endif /* SYSTEMCALLABSTRACTINTERFACE_H */
