/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "debug.h"
#include "controllerinterface.h"

ControllerInterface::ControllerInterface(const std::string &containerRoot):
    m_fifo(0), m_fifoPath(containerRoot + "rootfs/in_fifo")
{
}

ControllerInterface::~ControllerInterface()
{
}

bool ControllerInterface::startApp()
{
    if (m_fifo == 0)
        openFifo();

    write(m_fifo, "1\n", 2);

    return true;
}

bool ControllerInterface::shutdown()
{
    if (m_fifo == 0)
        openFifo();

    write(m_fifo, "2\n", 2);

    return true;
}

bool ControllerInterface::setEnvironmentVariable(const std::string &variable,
    const std::string &value)
{
    if (m_fifo == 0)
        openFifo();

    std::string command = "3 " + variable + " " + value;
    write(m_fifo, command.c_str(), command.size());

    return true;
}

bool ControllerInterface::systemCall(const std::string &cmd)
{
    if (m_fifo == 0)
        openFifo();

    int ret = write(m_fifo, cmd.c_str(), cmd.size());
    if (ret == -1) {
        perror("write: ");
        return false;
    }

    return true;
}

void ControllerInterface::openFifo()
{
    m_fifo = open(m_fifoPath.c_str(), O_WRONLY);
    if (m_fifo == -1)
        log_error("Error opening fifo: %s", strerror(errno));
}
