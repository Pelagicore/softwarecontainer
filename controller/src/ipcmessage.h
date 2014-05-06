/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H

#include <string>

#include "abstractcontroller.h"

class IPCMessage {
public:
    IPCMessage(AbstractController &controller);
    ~IPCMessage();

    /*! Call the appropriate method on Controller.
     *
     * The specified message will be parsed and the appropriate
     * method on Controller will be called with the arguments that were part
     * of the message passed over the IPC mechanism. This method returns
     * 'true' when the message has been passed to Controller and 'false' if
     * the message could not be processed for any reason.
     *
     * \param message A string with the complete message received over
     *          the IPC mechanism.
     *
     * \return true if the message was processed OK and passed to Controller,
     *          returns false if not.
     */
    bool handleMessage(const std::string &message);

private:
    void callSetEnvironmentVariable(const char *buf, int messageLength);
    void callSystemCall(const char *buf, int messageLength);

    AbstractController &m_controller;
};

#endif // IPCMESSAGE_H
