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

Controller::Controller(Glib::RefPtr<Glib::MainLoop> ml):
    m_ml(ml), m_pid(0)
{
}

Controller::~Controller()
{
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
        perror("setpgid error: ");
    }
}

void Controller::handleAppShutdownSlot(int pid, int exitCode) {
    std::cout << "handled app shutdown" << std::endl;
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
    std::cout << "Will run app now..." << std::endl;

    Glib::SignalChildWatch cw = Glib::signal_child_watch();

    std::vector<std::string> executeCommandVec;
    executeCommandVec = Glib::shell_parse_argv("/appbin/containedapp");
    sigc::slot<void> setupSlot = sigc::mem_fun(*this, &Controller::childSetupSlot);
    sigc::slot<void, int, int> shutdownSlot;
    shutdownSlot = sigc::mem_fun(*this, &Controller::handleAppShutdownSlot);
    try {
        Glib::spawn_async_with_pipes(".",
                                    executeCommandVec,
                                    Glib::SPAWN_DO_NOT_REAP_CHILD
                                        | Glib::SPAWN_SEARCH_PATH,
                                    setupSlot,
                                    &m_pid);
    } catch (const Glib::Error& ex) {
        // It's possible the spawn fails with an exception in which case we
        // catch it to do some cleanup instead of crashing hard.
        std::cout << "Error: " << ex.what() << std::endl;
    }

    cw.connect(shutdownSlot, m_pid);

    std::cout << "Started app with pid: " << "\"" << m_pid << "\"" << std::endl;

    return m_pid;
}

void Controller::killApp()
{
    std::cout << "Trying to kill: " << m_pid << std::endl;
    if (m_pid == 0) {
        std::cout << "WARNING: Trying to kill an app without previously having started one. "
            << "This is normal if this is a preloaded but unused container."
            << std::endl;
        shutdown();
        return;
    }

    // The negative pid makes kill send the signal to the whole process group
    int ret = kill(-m_pid, SIGINT);
    if (ret == -1) {
        perror("Error killing application: ");
        shutdown();
    }
}

void Controller::setEnvironmentVariable(const std::string &variable,
    const std::string &value)
{
    int ret = setenv(variable.c_str(), value.c_str(), 1);

    if (ret == -1) {
        perror("setenv: ");
        return;
    }

    std::cout << "Controller set \"" << variable << "=" << value << "\"" << std::endl;
}

void Controller::systemCall(const std::string &command)
{
    system(command.c_str());
    std::cout << "Controller executed command: " << command << std::endl;
}
