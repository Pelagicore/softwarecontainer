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

static const int BUF_SIZE = 1024;

FifoIPC::FifoIPC(IPCMessage &message):
    m_message(message), m_fifoPath(""), m_fifoCreated(false)
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

    if (m_fifoCreated == false) {
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

    char buf[BUF_SIZE];
    char c;
    bool shouldContinue = true;
    while (shouldContinue) {
        // Read next message in pipe, it should be null terminated
        int i = 0;
        do {
            int status = read(fd, &c, 1);
            if (status > 0) {
                buf[i++] = c;
            }
            // Look for end of message or end of storage buffer
        } while ((c != '\0') && (i != sizeof(buf) - 1));

        buf[i] = '\0';
        std::string messageString(buf);
        int status;
        shouldContinue = m_message.handleMessage(messageString, &status);
        if (status == -1) {
            // The message was not understood by IPCMessage
            std::cout << "Warning: IPC message to Controller was not sent" << std::endl;
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

    m_fifoCreated = true;

    return true;
}
