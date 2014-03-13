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
    IPCMessage(AbstractController *controller);
    ~IPCMessage();

    /*! Call the appropriate method on Controller.
     *
     * The specified message will be parsed and the appropriate
     * method on Controller will be called with the arguments that were part
     * of the message passed over the IPC mechanism. This method returns
     * 'true' when the message has been passed to Controller and indicates
     * that the IPC mechanism can continue passing messages. This method
     * returns 'false' to indicate that Controller will shut down and no
     * longer accept messages. The error flag is set if the message was not
     * understood and could not be passed to Controller.
     *
     * \param message A string with the complete message received over
     *          the IPC mechanism.
     * \param statusFlag A flag set to 0 on success and -1 if the message
     *          was not understood and could not be passed to Controller
     */
    bool send(const std::string &message, int *statusFlag);

private:
    void callSetEnvironmentVariable(const char *buf, int messageLength);
    void callSystemCall(const char *buf, int messageLength);

    AbstractController *m_controller;
};

#endif // IPCMESSAGE_H
