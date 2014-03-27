/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include "systemcallinterface.h"

SystemCallInterface::SystemCallInterface()
{
}

SystemCallInterface::~SystemCallInterface()
{
}

bool SystemCallInterface::makeCall(const std::string &cmd)
{
    bool success = (system(cmd.c_str()) == 0);
    return success;
}

bool SystemCallInterface::makeCall(const std::string &cmd, int &exitCode)
{
    exitCode = system(cmd.c_str());
    bool success = (exitCode == 0);
    return success;
}
