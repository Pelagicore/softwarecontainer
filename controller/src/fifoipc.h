/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef FIFOIPC_H
#define FIFOIPC_H

#include <string>

#include "abstractcontroller.h"

/*! FifoIPC is an implementation of an IPC based on a named pipe.
 *
 * The purpose of the IPC is to let Pelagicontain communicate with Controller.
 */
class FifoIPC
{
public:
    FifoIPC(AbstractController *controller);
    ~FifoIPC();

    /*! This call makes FifoIPC create a fifo and enter a loop to wait for
     *  input. Will return on error or when Controller has shut down the app.
     *
     * \param fifoPath A path to the fifo
     *
     * \return true if loop exited normally, false if there was any problem
     *          setting up or opening the fifo.
     */
    bool initialize(const std::string &fifoPath);

private:
    bool loop();
    bool createFifo();

    AbstractController *m_controller;
    std::string m_fifoPath;
    int m_fifo;
};

#endif //FIFOIPC_H
