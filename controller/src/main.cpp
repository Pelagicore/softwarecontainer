/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>
#include <glibmm.h>
#include <sys/stat.h>

#include "controller.h"
#include "socketipc.h"

#include "CommandLineParser.h"

#include "UNIXSignalGlibHandler.h"

#include "pelagicontain-common.h"

/**
 * "Controller" is meant to be the interface Pelagicontain uses to reach the
 * inside of the container.
 */

LOG_DECLARE_DEFAULT_CONTEXT(Controller_DefaultLogContext, "CON", "Main context");

int main(int argc, char **argv)
{
//	pelagicore::CommandLineParser parser("Pelagicontain controller", "<path>", PACKAGE_VERSION, "Description of the Pelagicontain controller");
//	parser.parse(argc, argv);

    log_info() << "In Controller";

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
    path += "ipc_socket";

//    if (!isDirectory("/gateways") ||
//        !isDirectory("/appbin") ||
//        !isDirectory("/apphome") ||
//        !isDirectory("/appshared"))
//    {
//        log_error() << "Expected directories not available in container, shutting down Controller";
//        return 1;
//    }

    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create();

    Controller controller(ml);
    IPCMessage message(controller);
    SocketIPC ipc(message);
    if (!ipc.initialize(path)) {
        log_error() << "Failed to connect to Pelagicontain";
        return 1;
    }

    // Start checking for messages from Pelagicontain from the IPC
    sigc::slot<bool> checkForMessagesSlot = sigc::mem_fun(&ipc, &SocketIPC::checkForMessages);
    sigc::connection conn = Glib::signal_timeout().connect(checkForMessagesSlot, 10);

    ml->run();

    return 0;
}
