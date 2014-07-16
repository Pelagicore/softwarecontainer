/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

#include "config.h"
#include "controllerinterface.h"

ControllerInterface::ControllerInterface(const std::string &gatewayDir):
    m_connected(false),
    m_socketPath(gatewayDir + "/ipc_socket"),
    m_running(false),
    m_listenSocket(0),
    m_connectionSocket(0),
    m_highestFd(0),
    m_timeout(Config::instance()->controllerConnectionTimeout())
{
}

ControllerInterface::~ControllerInterface()
{
    // Close both the sockets...
    if (m_connectionSocket) {
        if (close(m_connectionSocket) == -1) {
            log_error() << "close:" << strerror(errno);
        }
    }

    if (m_listenSocket) {
        if (close(m_listenSocket) == -1) {
            log_error() << "close listen:" << strerror(errno);
        }
    }

    // ...and unlink the file
    if (unlink(m_socketPath.c_str()) == -1) {
        log_error() << "unlink:" << strerror(errno);
    }
}

bool ControllerInterface::initialize()
{
    if ((m_listenSocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        log_error() << "socket:" << strerror(errno);
        return false;
    }

    struct sockaddr_un local;
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, m_socketPath.c_str());
    unlink(local.sun_path);

    int len = strlen(local.sun_path) * sizeof(local.sun_family);
    if (bind(m_listenSocket, (struct sockaddr *)&local, len) == -1) {
        log_error() << "bind:" << strerror(errno);
        return false;
    }

    if (listen(m_listenSocket, 1) == -1) {
        log_error() << "listen:" << strerror(errno);
        return false;
    }

    m_selectTimeout.tv_sec = m_timeout;
    m_selectTimeout.tv_usec = 0;

    if (!isControllerConnected()) {
        return false;
    }

    return true;
}

bool ControllerInterface::isControllerConnected()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_listenSocket, &readfds);

    log_debug() << "Waiting for Controller, timeout value is " << m_timeout << " seconds...";

    m_highestFd = m_listenSocket;

    struct timeval tv;
    tv.tv_sec = m_timeout;
    tv.tv_usec = 0;
    int ret = select(m_highestFd + 1, &readfds, 0, 0, &tv);
    if (ret == -1) {
        log_error() << "select:" << strerror(errno);
        return false;
    } else if (ret == 0) {
        log_error() << "select reached timeout";
        return false;
    }

    if (FD_ISSET(m_listenSocket, &readfds)) {
        socklen_t t = sizeof(m_remote);
        m_connectionSocket = accept(m_listenSocket, (struct sockaddr *)&m_remote, &t);
        if (m_connectionSocket == -1) {
            log_error() << "accept:" << strerror(errno);
            // TODO: We need to propagate failure here
            return false; // Disconnects the signal handler
        }

        log_debug() << "Accepted connection from Controller";
        m_connected = true;

        if (m_connectionSocket > m_highestFd) {
            m_highestFd = m_connectionSocket;
        }
        // We are connected, i.e. the Controller is there, now we should
        // be prepared to receive messages.
        sigc::slot<bool> checkForMessageSlot = sigc::mem_fun(this, &ControllerInterface::checkForMessage);
        sigc::connection conn = Glib::signal_timeout().connect(checkForMessageSlot, 10);
    }

    return true;
}

bool ControllerInterface::checkForMessage()
{
    char str[1024];
    memset(str, '\0', sizeof(char) * 1024);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_connectionSocket, &readfds);
    select(m_highestFd + 1, &readfds, 0, 0, &tv);

    if (FD_ISSET(m_connectionSocket, &readfds)) {
        int received = recv(m_connectionSocket, str, 100, 0);
        if (received == -1) {
            log_error() << "recv:" << strerror(errno);
            // TODO: Do we need to propagate failure here?
            // TODO: Should we continue waiting for messages after this?
            return false; // Disconnects the signal handler
        } else if (received == 0) {
            // Controller has shut down, there will be no more messages
            return false; // Disconnects the signal handler
        }

        log_debug() << "Pelagicontain received: " << str;
    }

    return true; // Continue to let the signal handler call this method
}

bool ControllerInterface::startApp()
{
    bool result = false;

    if (canSend()) {
        char msg[] = {'1', '\0'};

        log_debug("Sending: \"%s\"", msg);

        int ret = send(m_connectionSocket, msg, sizeof(msg), 0);
        if (ret == -1) {
            log_error() << "send:" << strerror(errno);
        } else {
            m_running = true;
            result = true;
        }
    }

    return result;
}

bool ControllerInterface::shutdown()
{
    bool result = false;

    if (canSend()) {
        char msg[] = {'2', '\0'};

        log_debug("Sending: \"%s\"", msg);

        int ret = send(m_connectionSocket, msg, sizeof(msg), 0);
        if (ret == -1) {
            log_error() << "send:" << strerror(errno);
        } else {
            m_running = false;
            result = true;
        }
    }

    return result;
}

bool ControllerInterface::setEnvironmentVariable(const std::string &variable,
                                                 const std::string &value)
{
    bool result = false;

    if (canSend()) {
        std::string command = "3 " + variable + " " + value;
        char msg[command.size() + 1];

        memcpy(msg, command.c_str(), command.size());
        msg[command.size()] = '\0';

        log_debug("Sending: \"%s\"", msg);

        int ret = send(m_connectionSocket, msg, strlen(msg) + 1, 0);
        if (ret == -1) {
            log_error() << "send:" << strerror(errno);
        } else {
            result = true;
        }
    }

    return result;
}

bool ControllerInterface::systemCall(const std::string &cmd)
{
    bool result = false;

    if (canSend()) {
        std::string command = "4 " + cmd;
        char msg[command.size() + 1];

        memcpy(msg, command.c_str(), command.size());
        msg[command.size()] = '\0';

        log_debug("Sending: \"%s\"", msg);

        int ret = send(m_connectionSocket, msg, strlen(msg) + 1, 0);
        if (ret == -1) {
            log_error() << "send:" << strerror(errno);
        } else {
            result = true;
        }
    }

    return result;
}

bool ControllerInterface::hasBeenStarted() const
{
    return m_running;
}

bool ControllerInterface::canSend()
{
    if (!m_connected) {
        log_error() << "No connection to Controller";
        return false;
    }

    fd_set sendfds;
    FD_ZERO(&sendfds);
    FD_SET(m_connectionSocket, &sendfds);

    int ret = select(m_highestFd + 1, 0, &sendfds, 0, &m_selectTimeout);
    if (ret == -1) {
        log_error() << "select:" << strerror(errno);
        return false;
    } else if (ret == 0) {
        log_error() << "select reached timeout";
        return false;
    }

    return FD_ISSET(m_connectionSocket, &sendfds) ? true : false;
}
