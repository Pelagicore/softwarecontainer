/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glibmm.h>

#include "controller.h"
#include "pelagicore-common.h"


Controller::Controller(Glib::RefPtr<Glib::MainLoop> ml):
    m_ml(ml), m_pid(0)
{
	adjustEnvironment();
}

Controller::~Controller()
{
}

void Controller::adjustEnvironment() {
#ifdef ADDITIONAL_LD_LIBRARY_PATH
	appendToEnvVariable(LD_LIBRARY_PATH_ENV_VARIABLE, STRINGIZE(ADDITIONAL_LD_LIBRARY_PATH));
#endif

#ifdef ADDITIONAL_PATH
	appendToEnvVariable(PATH_ENV_VARIABLE, STRINGIZE(ADDITIONAL_PATH));
#endif

}

bool Controller::killMainLoop() {
    if (m_ml) {
        m_ml->quit();
    }
    return true;
}

void Controller::childSetupSlot()
{
    int ret = setpgid(0, 0);
    if (ret == -1) {
    	log_error() << "setpgid error: " << strerror(errno);
    }
}

void Controller::handleAppShutdownSlot(int pid, int exitCode) {
    log_info() << "handled app shutdown" ;
    shutdown();
}

void Controller::shutdown()
{
    sigc::slot<bool> shutdownSlot;
    shutdownSlot = sigc::mem_fun(*this, &Controller::killMainLoop);
    Glib::signal_idle().connect(shutdownSlot);
}

int Controller::runApp()
{
    log_info() << "Will run app now..." ;

    std::vector<std::string> executeCommandVec;
    executeCommandVec = Glib::shell_parse_argv("/appbin/containedapp");
    sigc::slot<void> setupSlot = sigc::mem_fun(*this, &Controller::childSetupSlot);
    try {
        Glib::spawn_async_with_pipes(".",
                                    executeCommandVec,
                                    Glib::SPAWN_DO_NOT_REAP_CHILD
                                        | Glib::SPAWN_SEARCH_PATH,
                                    setupSlot,
                                    &m_pid);
    } catch (const Glib::Error &ex) {
        // It's a fatal error if app couldn't spawn, so we shut down.
        log_error() << ex.what();
        shutdown();
    }

    // We should only connect the watcher if the pid is valid
    if (m_pid) {
        sigc::slot<void, int, int> shutdownSlot;
        shutdownSlot = sigc::mem_fun(*this, &Controller::handleAppShutdownSlot);
        Glib::SignalChildWatch cw = Glib::signal_child_watch();
        cw.connect(shutdownSlot, m_pid);
        log_info() << "Started app with pid: " << m_pid;
    }

    return m_pid;
}

void Controller::killApp()
{
    log_info() << "Trying to kill pid: " << m_pid;
    if (m_pid == 0) {
        log_warn() << "Trying to kill an app without previously having started one. "
            "This is normal if this is a preloaded but unused container.";
        shutdown();
        return;
    }

    // The negative pid makes kill send the signal to the whole process group
    int ret = kill(-m_pid, SIGINT);
    if (ret == -1) {
    	log_error() << "Error killing application: " << strerror(errno);
        shutdown();
    }
}

void Controller::setEnvironmentVariable(const std::string &variable,
    const std::string &value)
{
    int ret = setenv(variable.c_str(), value.c_str(), 1);

    if (ret == -1) {
    	log_error() << "setenv: " << strerror(errno);
        return;
    }

    log_info() << "Controller set \"" << variable << "=" << value << "\"" ;
}

void Controller::systemCall(const std::string &command)
{
    system(command.c_str());
    log_info() << "Controller executed command: " << command ;
}
