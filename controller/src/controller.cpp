/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "errno.h"

#include "controller.h"

Controller::Controller():
    m_pid(0), m_fifoPath(""), m_fifo(0)
{
}

Controller::~Controller()
{
    int ret = unlink(m_fifoPath.c_str());
    if (ret == -1)
        std::cout << "Error removing fifo!" << std::endl;
}

bool Controller::initialize(const std::string &fifoPath)
{
    m_fifoPath = fifoPath;

    if (!fifoExists()) {
        bool success = createFifo();
        if (!success) {
            std::cout << "Could not create FIFO!" << std::endl;
            return false;
        }
    }

    loop();

    return true;
}

void Controller::loop()
{
    char c;
    int fd = open(m_fifoPath.c_str(), O_RDONLY);
    for (;;) {
        int status = read(fd, &c, 1);
        if (status > 0) {
            if (c == '1') {
                runApp();
                continue;
            } else if (c == '2') {
                killApp();
                // When app is shut down, we exit the loop and return
                // all the way back to main where we exit controller
                break;
            } else if (c == '\n') {
                // Ignore newlines
                continue;
            } else {
                std::cout << "Controller didn't understand that: " << c << std::endl;
            }
        }
    }
}

bool Controller::fifoExists()
{
    if (m_fifo != 0)
        return true;
    return false;
}

bool Controller::createFifo()
{
    int ret = mkfifo(m_fifoPath.c_str(), 0666);
    if ((ret == -1) && (errno == EEXIST)) {
        std::cout << "#### mkfifo: EEXIST" << std::endl;
        return true;
    } else if ((ret == -1) && (errno == EACCES)) {
        std::cout << "#### mkfifo: EACCES" << std::endl;
        return false;
    } else if ((ret == -1) && (errno == EDQUOT)) {
        std::cout << "#### mkfifo: EDQUOT" << std::endl;
        return false;
    } else if ((ret == -1) && (errno == ENAMETOOLONG)) {
        std::cout << "#### mkfifo: ENAMETOOLONG" << std::endl;
        return false;
    } else if ((ret == -1) && (errno == ENOSPC)) {
        std::cout << "#### mkfifo: ENOSPC" << std::endl;
        return false;
    } else if ((ret == -1) && (errno == EROFS)) {
        std::cout << "#### mkfifo: EROFS" << std::endl;
        return false;
    } else if ((ret == -1) && (errno == ENOENT)) {
        std::cout << "#### mkfifo: ENOENT" << std::endl;
        return false;
    } else if ((ret == -1) && (errno == ENOTDIR)) {
        std::cout << "#### mkfifo: ENOTDIR" << std::endl;
        return false;
    }

    m_fifo = 1;

    return true;
}

int Controller::runApp()
{
    std::cout << "Will run app now..." << std::endl;
    m_pid = fork();
    if (m_pid == 0) { // Child
        // This path to containedapp makes sense inside the container
        int ret = execlp("/deployed_app/containedapp", "containedapp", NULL);
        std::cout << "#### We should not be here! execlp: " << ret << " " << errno << std::endl;
        exit(0);
    } // Parent
    std::cout << "Started app with pid: " << "\"" << m_pid << "\"" << std::endl;
    return m_pid;
}

void Controller::killApp()
{
    std::cout << "Trying to kill: " << m_pid << std::endl;
    kill(m_pid, SIGINT);
    int status;
    waitpid(m_pid, &status, 0);
}
