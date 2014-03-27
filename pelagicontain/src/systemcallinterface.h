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
    SystemCallInterface() {}
    ~SystemCallInterface() {}

    virtual int makeCall(const std::string &cmd) { return system(cmd.c_str()); }
};

#endif /* SYSTEMCALLINTERFACE_H */
