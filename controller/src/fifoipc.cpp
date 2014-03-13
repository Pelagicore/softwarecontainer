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

FifoIPC::FifoIPC(IPCMessage *message):
    m_message(message), m_fifoPath(""), m_fifo(0)
{
}

FifoIPC::~FifoIPC()
{
    int ret = unlink(m_fifoPath.c_str());
    if (ret == -1) {
        perror("unlink: ");
    }
}

bool FifoIPC::initialize(const std::string &fifoPath)
{
    m_fifoPath = fifoPath;

    if (m_fifo == 0) {
        if (createFifo() == false) {
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
    char c;
    int i = 0;
    bool shouldContinue = false;
    for (;;) {
        memset(buf, 0, sizeof(buf));
        i = 0;
        do {
            int status = read(fd, &c, 1);
            if (status > 0) {
                buf[i] = c;
                ++i;
                if (i == sizeof(buf) - 1) {
                    break;
                }
            }
        } while (c != '\0');

        buf[sizeof(buf) - 1] = '\0';
        std::string messageString(buf);
        int status;
        shouldContinue = m_message->send(messageString, &status);
        if (status == -1) {
            std::cout << "Warning: IPC message to Controller was not sent" << std::endl;
        }
        if (shouldContinue == false) {
            break;
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
