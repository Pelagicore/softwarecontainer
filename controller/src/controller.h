/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <glibmm.h>

#include "abstractcontroller.h"
#include "log.h"

class Controller:
    public AbstractController
{
	LOG_DECLARE_CLASS_CONTEXT("CTRL", "Controller");

public:
    Controller(Glib::RefPtr<Glib::MainLoop> ml);
    ~Controller();

<<<<<<< df8ab6bef560ba4315c245778c1e8adaffa5313a
    virtual int runApp();
    /*!  
     *  Kill an app by sending SIGINT to it. If it has not shut down within
     *  five seconds, a SIGKILL is sent to it. 
     */
    virtual void killApp();
    virtual void setEnvironmentVariable(const std::string &variable,
                                        const std::string &value);
    virtual void systemCall(const std::string &command);
=======
    pid_t runApp() override;
    void killApp() override;
    void setEnvironmentVariable(const std::string &variable,
                                        const std::string &value) override;
    void systemCall(const std::string &command) override;
>>>>>>> Cleanup

private:
    void shutdown();
    bool killMainLoop();
    void childSetupSlot();
    void handleAppShutdownSlot(int pid, int exitCode);

    Glib::RefPtr<Glib::MainLoop> m_ml;
    pid_t m_pid;
};

#endif //CONTROLLER_H
