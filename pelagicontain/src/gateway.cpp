/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gateway.h"

ReturnCode Gateway::createSymLinkInContainer(const std::string &source, const std::string &destination)
{

    auto m_pid = getContainer().executeInContainer([ = ]() {
                log_debug() << "symlink " << source << " to " << destination;
                auto r = symlink( source.c_str(), destination.c_str() );
                if (r != 0) {
                    log_error() << "Can't create symlink " << source << " to " <<
                    destination;
                }
                return r;
            });

    assert(m_pid != 0);

    int status;
    waitpid(m_pid, &status, 0);
    return (status == 0) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}

bool Gateway::setConfig(const std::string &config)
{
    JSonElement rootElement(config);

    bool success = true;

    std::vector<JSonElement> elements;
    rootElement.read(elements);
    for (auto &element : elements) {
        if ( isError( readConfigElement(element) ) ) {
            success = false;
        }
    }

    return success;
}


pid_t Gateway::makePopenCall(const std::string &command,
        int *infp,
        int *outfp)
{
    int READ = 0;
    int WRITE = 1;

    int stdinfp[2], stdoutfp[2];
    pid_t pid;

    if (pipe(stdinfp) != 0 || pipe(stdoutfp) != 0) {
        log_error() << "Failed to open STDIN and STDOUT to dbus-proxy";
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

bool Gateway::makePcloseCall(pid_t pid, int infp, int outfp)
{
    if (infp > -1) {
        if (close(infp) == -1) {
            log_warning("Failed to close STDIN to dbus-proxy");
        }
    }
    if (outfp != -1) {
        if (close(outfp) == -1) {
            log_warning("Failed to close STDOUT to dbus-proxy");
        }
    }

    // the negative pid mekes it to kill the whole group, not only the shell
    int killed = kill(-pid, SIGKILL);

    return killed == 0;
}
