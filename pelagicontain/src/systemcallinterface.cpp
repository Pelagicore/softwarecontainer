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

bool SystemcallInterface::makePopenCall(const std::string &command, const std::string &type, FILE **fd)
{
    *fd = popen(command.c_str(), type.c_str());
    return (*fd == NULL);
}

bool SystemcallInterface::makePcloseCall(FILE **fd, int &exitCode)
{
    exitCode = pclose(*fd);
    return (exitCode == 0);
}
