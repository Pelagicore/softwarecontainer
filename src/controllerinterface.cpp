
#include <unistd.h>
#include <fcntl.h>

#include "controllerinterface.h"

bool ControllerInterface::startApp()
{
	int fd = open("/tmp/test/rootfs/in_fifo", O_WRONLY);
	write(fd, "1\n", 2);

	return true;
}

bool ControllerInterface::shutdown()
{
	int fd = open("/tmp/test/rootfs/in_fifo", O_WRONLY);
	write(fd, "2\n", 2);

	return true;
}