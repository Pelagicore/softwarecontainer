/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <unistd.h>
#include <fcntl.h>

#include "controllerinterface.h"

ControllerInterface::ControllerInterface(const std::string &containerRoot):
	m_containerRoot(containerRoot)
{
}

ControllerInterface::~ControllerInterface()
{
}

bool ControllerInterface::startApp()
{
	std::string fifoPath = m_containerRoot + "rootfs/in_fifo";
	int fd = open(fifoPath.c_str(), O_WRONLY);
	write(fd, "1\n", 2);

	return true;
}

bool ControllerInterface::shutdown()
{
	std::string fifoPath = m_containerRoot + "rootfs/in_fifo";
	int fd = open(fifoPath.c_str(), O_WRONLY);
	write(fd, "2\n", 2);

	return true;
}
