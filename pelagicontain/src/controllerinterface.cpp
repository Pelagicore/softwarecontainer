/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

#include "debug.h"
#include "controllerinterface.h"

ControllerInterface::ControllerInterface(const std::string &gatewayDir):
	m_fifo(0), m_fifoPath(gatewayDir + "/in_fifo")
{
}

ControllerInterface::~ControllerInterface()
{
}

bool ControllerInterface::startApp()
{
    if (m_fifo == 0)
        if (openFifo() == false)
            return false;

    char msg[] = {'1', '\0'};
    int ret = write(m_fifo, msg, sizeof(msg));
    if (ret == -1) {
        log_error("write: %s", strerror(errno));
        return false;
    }

    return true;
}

bool ControllerInterface::shutdown()
{
    if (m_fifo == 0)
        if (openFifo() == false)
            return false;

    char msg[] = {'2', '\0'};
    int ret = write(m_fifo, msg, sizeof(msg));
    if (ret == -1) {
        log_error("write: %s", strerror(errno));
        return false;
    }

    return true;
}

bool ControllerInterface::setEnvironmentVariable(const std::string &variable,
    const std::string &value)
{
    if (m_fifo == 0)
        if (openFifo() == false)
            return false;

    std::string command = "3 " + variable + " " + value;
    int ret = write(m_fifo, command.c_str(), command.size() + 1);
    if (ret == -1) {
        log_error("write: %s", strerror(errno));
        return false;
    }

    return true;
}

bool ControllerInterface::systemCall(const std::string &cmd)
{
    if (m_fifo == 0)
        if (openFifo() == false)
            return false;

    std::string command = "4 " + cmd;
    int ret = write(m_fifo, command.c_str(), command.size() + 1);
    if (ret == -1) {
        log_error("write: %s", strerror(errno));
        return false;
    }

    return true;
}

bool ControllerInterface::openFifo()
{
    /*
    while (access (m_fifoPath.c_str(), F_OK) == -1) {
        log_error (std::string("FIFO ("+m_fifoPath+") not available.."
        " Spinning..").c_str());
    }*/

    m_fifo = open(m_fifoPath.c_str(), O_WRONLY);
    if (m_fifo == -1) {
        log_error("open: %s erro=%s", m_fifoPath.c_str(), strerror(errno));
        return false;
    }

    return true;
}
