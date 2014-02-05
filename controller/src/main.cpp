
#include <iostream>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "errno.h"

/**
 * "Controller" is meant to be the interface Pelagicontain uses to reach the
 * inside of the container. This implementation is just a stub to support the
 * basic flow between components, consider it a test. Must be made more useful and robust!
 */

int runApp();
void killApp();
void shutdown();

pid_t pid;

int main(int argc, char **argv)
{
	std::cout << "In Controller" << std::endl;

	/** The first command line arg to controller can be the path to where the
	 *  FIFO file is. This is set to /deployed_app/ if it is launched with no
	 *  argument (e.g. called by Pelagicontain), but for testing reasons it's
	 *  convenient if the real path can be specified instead.
	 */
	std::string path;
	if (argc == 2)
		path = std::string(argv[1]);
	else
		path = std::string("/deployed_app/");
	path += "in_fifo";

	char c;
	int fd = open(path.c_str(), O_RDONLY);
	for (;;) {
		int status = read(fd, &c, 1);
		if (status > 0) {
			switch (c) {
			case '1':
				runApp();
				continue;
			case '2':
				killApp();
				continue;
			case '\n':
				// Ignore newline
				continue;
			default:
				std::cout << "Controller didn't understand that" << std::endl;
			}
		}
	}
	return 0;
}

int runApp()
{
	std::cout << "Will run app now..." << std::endl;
	pid = fork();
	if (pid == 0) { // Child
		// This path to containedapp makes sense inside the container
		int ret = execlp("/deployed_app/containedapp", "containedapp", NULL);
		std::cout << "#### We should not be here! execlp: " << ret << " " << errno << std::endl;
		exit(0);
	} // Parent
	std::cout << "#### pid: " << "\"" << pid << "\"" << std::endl;
	return pid;
}

void killApp()
{
	std::cout << "Trying to kill " << pid << std::endl;
	kill(pid, SIGINT);
	int status;
	waitpid(pid, &status, 0);
	if (WIFEXITED(status)) {
		std::cout << "Wait status for pid " << pid << ": " << WEXITSTATUS(status) << std::endl;
	}
	shutdown();
}

void shutdown()
{
	exit(0);
}
