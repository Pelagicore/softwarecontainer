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

#include "controller.h"

Controller::Controller():
    m_pid(0)
{
}

Controller::~Controller()
{
}

int Controller::runApp()
{
    std::cout << "Will run app now..." << std::endl;

    m_pid = fork();
    if (m_pid == -1) {
        perror("Error starting application: ");
        return m_pid;
    }

    if (m_pid == 0) { // Child
        // This path to containedapp makes sense inside the container
        int ret = execlp("/appbin/containedapp", "containedapp", NULL);
        if (ret == -1)
            perror("exec error: ");
        exit(1);
    } // Parent
    std::cout << "Started app with pid: " << "\"" << m_pid << "\"" << std::endl;

    return m_pid;
}

void Controller::killApp()
{
    std::cout << "Trying to kill: " << m_pid << std::endl;
    if (m_pid == 0) {
        std::cout << "Warning: Trying to kill an app without previously having started one" << std::endl;
        return;
    }

    int ret = kill(m_pid, SIGINT);
    if (ret == -1) {
        perror("Error killing application: ");
    } else {
        int status;
        waitpid(m_pid, &status, 0);
        if (WIFEXITED(status))
            std::cout << "Controller: wait: app exited (WIFEXITED)" << std::endl;
        if (WIFSIGNALED(status))
            std::cout << "Controller: wait: app shut down by signal: " <<
                WTERMSIG(status) << " (WIFSIGNALED)" << std::endl;
    }
}

void Controller::setEnvironmentVariable(const std::string &variable,
    const std::string &value)
{
    std::string command = variable + "=" + value;
    int ret = putenv((char *)command.c_str());
    if (ret != 0)
        perror("putenv: ");
    std::cout << "Controller set \"" << variable << "\" to \"" << value << "\"" << std::endl;
}

void Controller::systemCall(const std::string &command)
{
    system(command.c_str());
    std::cout << "Controller executed command: " << command << std::endl;
}
