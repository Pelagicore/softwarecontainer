/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <iostream>

#include "controller.h"
#include "fifoipc.h"

/**
 * "Controller" is meant to be the interface Pelagicontain uses to reach the
 * inside of the container. This implementation is just a stub to support the
 * basic flow between components, consider it a test. Must be made more useful and robust!
 */

int main(int argc, char **argv)
{
    std::cout << "In Controller" << std::endl;

    /** The first command line arg to controller can be the path to where the
     *  FIFO file will be created. This is set to /deployed_app/ if it is
     *  launched with no argument (e.g. called by Pelagicontain), but for
     *  testing reasons it's convenient if the real path can be specified instead.
     */
    std::string path;
    if (argc == 2)
            path = std::string(argv[1]);
    else
            path = std::string("/deployed_app/");
    path += "in_fifo";

    Controller controller;
    FifoIPC ipc(&controller);
    ipc.initialize(path);

    return 0;
}
