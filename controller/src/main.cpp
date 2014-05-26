/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>
#include <glibmm.h>
#include <poll.h>

#include "controller.h"
#include "fifoipc.h"

/**
 * "Controller" is meant to be the interface Pelagicontain uses to reach the
 * inside of the container. This implementation is just a stub to support the
 * basic flow between components, consider it a test. Must be made more useful and robust!
 */

LOG_DECLARE_DEFAULT_CONTEXT(Controller_DefaultLogContext, "CON", "Main context");

int main(int argc, char **argv)
{
    log_info() << "In Controller" ;

    /** The first command line arg to controller can be the path to where the
     *  FIFO file will be created. This is set to /gateways/ if it is
     *  launched with no argument (e.g. called by Pelagicontain), but for
     *  testing reasons it's convenient if the real path can be specified instead.
     */
    std::string path;
    if (argc == 2)
        path = std::string(argv[1]);
    else
        path = std::string("/gateways/");
    path += "in_fifo";

    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create();

    Controller controller(ml);
    IPCMessage message(controller);
    FifoIPC ipc(message);
    ipc.initialize(path);

    sigc::slot<bool> fifoSlot = sigc::mem_fun(&ipc, &FifoIPC::loop);
    sigc::connection conn = Glib::signal_timeout().connect(fifoSlot, 100);

    ml->run();

    return 0;
}
