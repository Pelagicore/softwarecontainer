/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "errno.h"
#include <cstring>

#include "fifoipc.h"

FifoIPC::FifoIPC(AbstractController *controller):
    m_controller(controller), m_fifoPath(""), m_fifo(0)
{
}

FifoIPC::~FifoIPC()
{
    int ret = unlink(m_fifoPath.c_str());
    if (ret == -1)
        std::cout << "Error removing fifo!" << std::endl;
}

bool FifoIPC::initialize(const std::string &fifoPath)
{
    m_fifoPath = fifoPath;

    if (m_fifo == 0) {
        bool created = createFifo();
        if (!created) {
            std::cout << "Could not create FIFO!" << std::endl;
            return false;
        }
    }

    return loop();
}

bool FifoIPC::loop()
{
    int fd = open(m_fifoPath.c_str(), O_RDONLY);
    if (fd == -1) {
        perror("Error opening fifo: ");
        return false;
    }

    char buf[1024];
    for (;;) {
        memset(buf, 0, sizeof(buf));
        // Leave the last element for null termination
        int status = read(fd, buf, sizeof(buf)-1);
        if (status > 0) {
            std::cout << buf << std::endl;
            if (buf[0] == '1') {
                m_controller->runApp();
                continue;
            } else if (buf[0] == '2') {
                m_controller->killApp();
                // When app is shut down, we exit the loop and return
                // all the way back to main where we exit the program
                break;
            } else if (buf[0] == '\n') {
                // Ignore newlines
                continue;
            } else {
                buf[sizeof(buf)-1] = '\0';
                m_controller->systemCall(std::string(buf));
            }
        }
    }

    return true;
}

bool FifoIPC::createFifo()
{
    int ret = mkfifo(m_fifoPath.c_str(), 0666);
    // All errors except for when fifo already exist are bad
    if ((ret == -1) && (errno != EEXIST)) {
        perror("Error creating fifo: ");
        return false;
    }

    m_fifo = 1;

    return true;
}
