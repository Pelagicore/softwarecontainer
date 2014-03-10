/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "abstractcontroller.h"

class Controller:
    public AbstractController
{
public:
    Controller();
    ~Controller();

    virtual int runApp();
    virtual void killApp();
    virtual void systemCall(const std::string &command);

private:
    pid_t m_pid;
};

#endif //CONTROLLER_H
