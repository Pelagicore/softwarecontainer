/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <dbus-c++/dbus.h>

#include "dbusadaptor.h"
#include <iostream>
#include <signal.h>
#include <cstdlib>

/*!
 * This application is used to provide an app to run in a container while
 * running the Pelagicontain component tests.
 *
 * Currently there is only a loop and a signal handler, when there is support
 * for D-Bus services inside the containers, the application should also
 * exopose an interface on D-Bus.
 */

// class ContainedApp :
//      public com::pelagicore::test::ContainedApp_adaptor,
//      public DBus::IntrospectableAdaptor,
//      public DBus::ObjectAdaptor
// {
// public:
//      ContainedApp (DBus::Connection &conn) :
//              DBus::ObjectAdaptor(conn, "/com/pelagicore/test/ContainedApp")
//      {
//              std::cout << "Inside ContainedApp constructor" << std::endl;
//      }
//      virtual std::string Echo(const std::string& argument) {
//              std::cout << "ContainedApp says: " << argument << std::endl;
//              return "ContainedApp says: " + argument;
//      }
// };

void signalHandler(int s)
{
    std::cout << "Caught signal " << s << std::endl;
    exit(0);
}

int main(int argc, char **argv)
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    for (;; ) ;

    //  DBus::BusDispatcher dispatcher;
    //
    //  DBus::default_dispatcher = &dispatcher;
    //  DBus::Connection bus = DBus::Connection::SessionBus();
    //  ContainedApp app(bus);
    //
    //  bus.request_name("com.pelagicore.test.ContainedApp");
    //
    //  dispatcher.enter();
}
