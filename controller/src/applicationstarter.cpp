#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <dirent.h>
#include <unistd.h>

#include "log.h"

LOG_DECLARE_DEFAULT_CONTEXT(mainLogcontext, "STAR", "")

enum class ReturnCode {
	OK, ERROR
};

ReturnCode connectToServer(const char* uds_socket_path) {

	int fileDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fileDescriptor == -1) {
		log_warning()
		<< "Failed to create socket";
		return ReturnCode::ERROR;
	}

	struct sockaddr_un remote;

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, uds_socket_path);

	if (::connect(fileDescriptor, (struct sockaddr*) &remote,
			strlen(remote.sun_path) + sizeof(remote.sun_family)) == -1) {
		log_warning()
		<< "Failed to connect to the daemon via socket " << uds_socket_path;
		return ReturnCode::ERROR;
	}

	return ReturnCode::OK;
}

int main(int argc, char **argv) {

	if (argc > 3) {
		const char* uds_socket_path = argv[1];
		log_info()
		<< "Connecting to socket" << uds_socket_path;

		if (connectToServer(uds_socket_path) == ReturnCode::OK) {

			std::vector<char*> parameters;
			for (int i = 2; i < argc; i++)
				parameters.push_back(argv[i]);

			log_info()
			<< "Exec " << parameters;
			execv(argv[2], argv + 2);
		}
	} else {
		log_info()
		<< "no arg";
	}

}
