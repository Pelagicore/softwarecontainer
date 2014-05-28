/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include "systemcallinterface.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>


SystemcallInterface::SystemcallInterface()
{
}

SystemcallInterface::~SystemcallInterface()
{
}

bool SystemcallInterface::makeCall(const std::string &cmd)
{
    bool success = (system(cmd.c_str()) == 0);
    return success;
}

bool SystemcallInterface::makeCall(const std::string &cmd, int &exitCode)
{
    exitCode = system(cmd.c_str());
    return (exitCode == 0);
}

pid_t SystemcallInterface::makePopenCall(const std::string &command,
                                         int *infp,
                                         int *outfp)
{
    int READ = 0;
    int WRITE = 1;

    int stdinfp[2], stdoutfp[2];
    pid_t pid;

    if (pipe(stdinfp) != 0 || pipe(stdoutfp) != 0) {
        log_error("Failed to open STDIN and STDOUT to dbus-proxy");
        return -1;
    }

    pid = fork();

    if (pid < 0) {
        return pid;
    } else if (pid == 0) {
        close(stdinfp[WRITE]);
        dup2(stdinfp[READ], READ);
        close(stdoutfp[READ]);
        dup2(stdoutfp[WRITE], WRITE);

        // Set group id to the same as pid, that way we can kill the
        // shells children on close.
        setpgid(0, 0);

        execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
    	log_error("execl") << strerror(errno);
        exit(1);
    }

    if (infp == NULL) {
        close(stdinfp[WRITE]);
    } else {
        *infp = stdinfp[WRITE];
    }

    if (outfp == NULL) {
        close(stdoutfp[READ]);
    } else {
        *outfp = stdoutfp[READ];
    }

    return pid;
}

bool SystemcallInterface::makePcloseCall(pid_t pid, int infp, int outfp)
{
    if (infp > -1) {
        if (close(infp) == -1) {
            log_warning ("Failed to close STDIN to dbus-proxy");
        }
    }
    if (outfp != -1) {
        if (close(outfp) == -1) {
            log_warning ("Failed to close STDOUT to dbus-proxy");
        }
    }

    // the negative pid mekes it to kill the whole group, not only the shell
    int killed = kill(-pid, SIGKILL);

    return killed == 0;
}
