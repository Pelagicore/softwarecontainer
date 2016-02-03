/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gateway.h"
#include <fcntl.h>

// TODO: Move this to somewhere reasonable
constexpr const char *Gateway::XDG_RUNTIME_DIR_VARIABLE_NAME;

bool Gateway::setConfig(const std::string &config)
{
    bool success = true;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);
    if (!root) {
        log_error() << "Could not parse config: " << error.text;
        return false;
    }

    if (json_is_array(root)) {
        for(size_t i = 0; i < json_array_size(root); i++) {
            json_t *element = json_array_get(root, i);
            if (isError(readConfigElement(element))) {
                log_warning() << "Could not read config element";
                success = false;
            }
        }
    }
    return success;
}

/*
 * Opens a channel to some command and captures the input file descriptor and
 * pid for future writing and control. Output from the call is directed
 * towards /dev/null
 */
ReturnCode Gateway::makePopenCall(const std::string &command, int &infp, pid_t &pid)
{
    static constexpr int READ = 0;
    static constexpr int WRITE = 1;

    int stdinfp[2];
    if (pipe(stdinfp) != 0) {
        log_error() << "Failed to open STDIN to " << command;
        return ReturnCode::FAILURE;
    }

    pid = fork();

    if (pid < 0) {
        return ReturnCode::FAILURE;
    } else if (pid == 0) {
        close(stdinfp[WRITE]);
        dup2(stdinfp[READ], READ);

        int nullfd = open("/dev/null", O_WRONLY);
        if (nullfd == -1) {
            log_error() << "Could not open /dev/null: " << strerror(errno);
        }
        dup2(nullfd, WRITE);

        // Set group id to the same as pid, that way we can kill the shells children on close.
        setpgid(0, 0);

        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        log_error() << "execl : " << strerror(errno);
        close(nullfd);
        exit(1);
    }

    infp = stdinfp[WRITE];
    return ReturnCode::SUCCESS;
}

/*
 * Closes the file pointer and kills the pid
 */
bool Gateway::makePcloseCall(pid_t pid, int infp)
{
    if (infp > -1) {
        if (close(infp) == -1) {
            log_warning() << "Failed to close STDIN";
        }
    }

    // the negative pid makes it kill the whole group, not only the shell
    int killed = kill(-pid, SIGKILL);
    return (killed == 0);
}
