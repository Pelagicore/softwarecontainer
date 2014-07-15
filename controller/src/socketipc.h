/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef SOCKETIPC_H
#define SOCKETIPC_H

#include <string>
#include <sys/socket.h>
#include <sys/un.h>

#include "ipcmessage.h"
#include "log.h"

/*! SocketIPC is an implementation of an IPC based on a Unix domain socket.
 *
 * The purpose of the IPC is to let Pelagicontain communicate with Controller.
 */
class SocketIPC
{
    LOG_DECLARE_CLASS_CONTEXT("IPC", "Controller");

public:
    SocketIPC(IPCMessage &message);
    ~SocketIPC();

    /*! Connects to Pelagicontain through the IPC socket set up by Pelagicontain.
     *  Returns true if the connection could be estabished, false on error. If
     *  false is returned it is considered a fatal error and Controller should
     *  exist as soon as possible.
     *
     * \param socketPath An absolute path to the IPC socket
     *
     * \return true or false
     */
    bool initialize(const std::string &socketPath);

    /*! Checks if there are any messages from Pelagicontain to recieve.
     *  A timeout signal handler calls this method until it returns false, which
     *  is only done when an error occurs. Will be called on timeout as long
     *  as it returns true.
     *
     * \return true or false
     */
    bool checkForMessages();

private:
    IPCMessage &m_message;
    std::string m_socketPath;
    int m_socket;
    int m_bufferSize;
};

#endif //SOCKETIPC_H
