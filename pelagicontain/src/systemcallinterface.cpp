/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include "systemcallinterface.h"

SystemcallInterface::SystemcallInterface()
{
}

SystemcallInterface::~SystemcallInterface()
{
}

bool SystemcallInterface::makeCall(const std::string &cmd)
{
    bool success = (system(cmd.c_str()) == 0);
    return success;
}

bool SystemcallInterface::makeCall(const std::string &cmd, int &exitCode)
{
    exitCode = system(cmd.c_str());
    return (exitCode == 0);
}
