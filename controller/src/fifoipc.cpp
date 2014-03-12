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

// The IPC protocol prefixes messages with two chars not part of the message
static const int OFFSET = 2;

FifoIPC::FifoIPC(AbstractController *controller):
    m_controller(controller), m_fifoPath(""), m_fifo(0)
{
}

FifoIPC::~FifoIPC()
{
    int ret = unlink(m_fifoPath.c_str());
    if (ret == -1)
        perror("unlink: ");
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
    for (;;) {
        memset(buf, 0, sizeof(buf));
        i = 0;
        do {
            int status = read(fd, &c, 1);
            if (status > 0) {
                buf[i] = c;
                ++i;
            }
        } while (c != '\0');

        if (buf[0] == '1') {
            m_controller->runApp();
            continue;
        } else if (buf[0] == '2') {
            m_controller->killApp();
            // When app is shut down, we exit the loop and return
            // all the way back to main where we exit the program
            break;
        } else if (buf[0] == '3') {
            char variable[1024];
            memset(variable, 0, sizeof(variable));

            char value[1024];
            memset(value, 0, sizeof(value));

            // Find the variable and the value
            for (unsigned i = OFFSET; i < sizeof(buf); ++i) {
                if (buf[i] == ' ') {
                    // We're between the variable and the value
                    int separator = i;
                    strncpy(variable, buf + OFFSET, separator - OFFSET);
                    strncpy(value, buf + OFFSET + separator - 1, sizeof(buf));
                    break;
                }
            }

            variable[sizeof(variable) - 1] = '\0';
            std::string variableString(variable);

            value[sizeof(value) - 1] = '\0';
            std::string valueString(value);

            m_controller->setEnvironmentVariable(variableString, valueString);
        } else if (buf[0] == '4') {
            char command[1024];
            memset(command, 0, sizeof(command));

            strncpy(command, buf + OFFSET, sizeof(buf));

            command[sizeof(command) - 1] = '\0';
            m_controller->systemCall(std::string(command));
        } else {
            return false;
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
