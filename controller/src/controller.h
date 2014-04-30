/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <glibmm.h>

#include "abstractcontroller.h"

class Controller:
    public AbstractController
{
public:
    Controller(Glib::RefPtr<Glib::MainLoop> ml);
    ~Controller();

    virtual int runApp();
    virtual void killApp();
    virtual void setEnvironmentVariable(const std::string &variable,
                                        const std::string &value);
    virtual void systemCall(const std::string &command);

private:
    void shutdown();
    bool killMainLoop();
    void childSetupSlot();
    void handleAppShutdownSlot(int pid, int exitCode);

    Glib::RefPtr<Glib::MainLoop> m_ml;
    pid_t m_pid;
};

#endif //CONTROLLER_H
