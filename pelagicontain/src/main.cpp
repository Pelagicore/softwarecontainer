/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "CommandLineParser.h"
#include "debug.h"
#include "paminterface.h"
#include "pelagicontain.h"
#include "pelagicontaintodbusadapter.h"
#include "dbusmainloop.h"
#include "generators.h" /* used for gen_ct_name */
#include "pulsegateway.h"
#include "networkgateway.h"
#include "dbusgateway.h"
#include <sys/stat.h>
#include "systemcallinterface.h"
#include "devicenodegateway.h"
#include "MainLoopApplication.h"
#include "GlibIO.h"

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

#ifndef CONFIG
    #error Must define CONFIG; path to configuration file (/etc/pelagicontain?)
#endif

bool event() {
    log_debug("This is the event firing!");
    return true;
}

int main(int argc, char **argv)
{

    const char *summary = "Pelagicore container utility. "
                          "Requires an absolute path to the container root, "
                          "the command to run inside the container and "
                          "an alphanumerical cookie string as first, second and"
                          "third argument respectively";
    const char *paramsDescription = "[container root directory (abs path)] "
                                    "[command] [cookie]";

    pelagicore::CommandLineParser commandLineParser(summary,
                                                    paramsDescription,
                                                    PACKAGE_VERSION,
                                                    "");

    std::string containerRoot;
    std::string containedCommand;
    std::string cookie;
    const char* configFilePath = CONFIG;
    commandLineParser.addOption(configFilePath,
                                "with-config-file",
                                'c',
                                "Config file");

    if (commandLineParser.parse(argc, argv)) {
        return -1;
    }

    if (argc < 4) {
        log_error("Invalid arguments");
        commandLineParser.printHelp();
        return -1;
    } else {
        containerRoot = std::string(argv[1]);
        containedCommand = std::string(argv[2]);
        cookie = std::string(argv[3]);
    }

    if (containerRoot.c_str()[0] != '/') {
        log_error("Path to container root must be absolute");
        commandLineParser.printHelp();
        return -1;
    }

    std::string containerName = gen_ct_name();
    std::string containerConfig(configFilePath);

    // Create gateway directory for container in containerRoot/gateways.
    // This dir will contain various sockets etc acting as gateways
    // in/out of the container.
    std::string containerDir = containerRoot + "/" + containerName;
    std::string gatewayDir = containerDir + "/gateways";
    if (mkdir(containerDir.c_str(), S_IRWXU) == -1) {
        log_error("Could not create container directory %s, %s.",
                  containerDir.c_str(),
                  strerror(errno));
        exit(-1);
    }
    if (mkdir(gatewayDir.c_str(), S_IRWXU) == -1) {
        log_error("Could not create gateway directory %s, %s.",
                  gatewayDir.c_str(),
                  strerror(errno));
        exit(-1);
    }

    pelagicore::MainLoopApplication ml;
    pelagicore::GlibIdleCallback cb (event);
    cb.activate(900);

    { // Create a new scope so that we can do a clean up after dtors
        DBus::Glib::BusDispatcher dispatcher;
        dispatcher.attach(ml.getMainContext());
        DBus::default_dispatcher = &dispatcher;
        DBus::Connection bus = DBus::Connection::SessionBus();

        /* Pelagicontain needs an interface to the mainloop so it can
         * exit it when we are to clean up and shut down
         */
        DBusMainloop dbusmainloop(&dispatcher);

        /* The request_name call does not return anything but raises an
         * exception if the name cannot be requested.
         */
        std::string name = "com.pelagicore.Pelagicontain" + cookie;
        bus.request_name(name.c_str());

        PAMInterface pamInterface(bus);
        ControllerInterface controllerInterface(gatewayDir);
        SystemcallInterface systemcallInterface;
        Pelagicontain pelagicontain(&pamInterface,
                                    &dbusmainloop,
                                    &controllerInterface,
                                    cookie);

        std::string objectPath = "/com/pelagicore/Pelagicontain";

        PelagicontainToDBusAdapter pcAdapter(bus, objectPath, pelagicontain);

        pelagicontain.addGateway(new NetworkGateway(&controllerInterface,
                                                    &systemcallInterface));

        pelagicontain.addGateway(new PulseGateway(gatewayDir, containerName,
                                                  &controllerInterface));

        pelagicontain.addGateway(new DeviceNodeGateway(&controllerInterface));

        pelagicontain.addGateway(new DBusGateway(&controllerInterface,
                                                 &systemcallInterface,
                                                 DBusGateway::SessionProxy,
                                                 gatewayDir,
                                                 containerName));

        pelagicontain.addGateway(new DBusGateway(&controllerInterface,
                                                 &systemcallInterface,
                                                 DBusGateway::SystemProxy,
                                                 gatewayDir,
                                                 containerName));

        pid_t pcPid = pelagicontain.preload(containerName,
                                            containerConfig,
                                            containerRoot,
                                            containedCommand);

        log_debug("Started Pelagicontain with PID: %d", pcPid);

       // dbusmainloop.enter();
        ml.run();
        log_debug("Exited dbusmainloop.");
    }

    // remove instance specific dirs again
    if (rmdir(gatewayDir.c_str()) == -1) {
        log_error("Cannot delete dir %s, %s",
                  gatewayDir.c_str(),
                  strerror(errno));
    }

    if (rmdir(containerDir.c_str()) == -1) {
        log_error("Cannot delete dir %s, %s",
                  gatewayDir.c_str(),
                  strerror(errno));
    }
}
