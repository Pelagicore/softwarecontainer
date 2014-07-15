/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H

#include <string>
#include "log.h"

#include "abstractcontroller.h"

class IPCMessage {
    LOG_DECLARE_CLASS_CONTEXT("MSG", "IPC Message");

public:
    IPCMessage(AbstractController &controller);
    ~IPCMessage();

    /*! Parse and pass all messages in buffer to Controller
     *
     *  The messages in the buffer must be null terminated and the \c length
     *  argument can not be greater than the size of the buffer. Returns \c true
     *  if all messages found in buffer were passed to Controller, \c false if
     *  one or more messages could not be passed to Controller, or if the
     *  \c length argument is greater than allowed.
     */
    bool handleMessage(const char buf[], int length);

private:
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
    bool dispatchMessage(const char buf[]);

    void callSetEnvironmentVariable(const char buf[], int messageLength);

    void callSystemCall(const char buf[], int messageLength);

    AbstractController &m_controller;
    int m_bufferSize;
};

#endif // IPCMESSAGE_H
