/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <glibmm.h>
#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

#include <signal.h>
#include <unistd.h>
#include "CommandLineParser.h"
#include "log.h"
#include "paminterface.h"
#include "pelagicontain.h"
#include "pelagicontaintodbusadapter.h"
#include "generators.h" /* used for gen_ct_name */

#include <sys/stat.h>
#include "systemcallinterface.h"

#ifdef ENABLE_PULSEGATEWAY
#include "pulsegateway.h"
#endif

#ifdef ENABLE_NETWORKGATEWAY
#include "networkgateway.h"
#endif

#ifdef ENABLE_DBUSGATEWAY
#include "dbusgateway.h"
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
#include "devicenodegateway.h"
#endif

#ifdef ENABLE_CGROUPSGATEWAY
#include "cgroupsgateway.h"
#endif

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_DEFAULT_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

#ifndef CONFIG
    #error Must define CONFIG; path to configuration file (/etc/pelagicontain?)
#endif

/* These has to be global in order to be reachable from the signal handler */
static Pelagicontain *pelagicontain;
static pid_t pcPid;
static std::string containerDir;

/**
 * Remove the dirs created by main when we cleanup
 */
void removeDirs()
{
    std::string gatewayDir = containerDir + "/gateways";
    if (rmdir(gatewayDir.c_str()) == -1) {
        log_error("Cannot delete dir %s, %s",
                  gatewayDir.c_str(),
                  strerror(errno));
    }

    if (rmdir(containerDir.c_str()) == -1) {
        log_error("Cannot delete dir %s, %s",
                  containerDir.c_str(),
                  strerror(errno));
    }
}

void signalHandler(int signum)
{
    log_debug() << "caught signal " << signum;
    if (!pcPid) {
        if (pelagicontain) {
            // pelagicontain might not have been initialized (preloaded), so we can't just
            // run shutdownContainer. But in the case it is initialized we DO want to run
            // it. Unfortunately, there is currently no way to check that.
            // pelagicontain->shutdownContainer();
            delete pelagicontain;
            removeDirs();
        }
        exit(signum);
    } else {
        // But if pcPid is set, then it is definately initialized
        // This will shutdown pelagicontain and make it exit the main loop
        // back to main() which will take care of the rest of the cleanup
        pelagicontain->shutdown();
    }
}

int main(int argc, char **argv)
{
    // Register signalHandler with signals
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

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

    std::string containerName = Generator::gen_ct_name();
    std::string containerConfig(configFilePath);

    // Create gateway directory for container in containerRoot/gateways.
    // This dir will contain various sockets etc acting as gateways
    // in/out of the container.
    containerDir = containerRoot + "/" + containerName;
    std::string gatewayDir = containerDir + "/gateways";
    if (mkdir(containerDir.c_str(), S_IRWXU) == -1) {
        log_error("Could not create container directory %s, %s.",
                  containerDir.c_str(),
                  strerror(errno));
        return -1;
    }
    if (mkdir(gatewayDir.c_str(), S_IRWXU) == -1) {
        log_error("Could not create gateway directory %s, %s.",
                  gatewayDir.c_str(),
                  strerror(errno));
        return -1;
    }

    // Make sure path ends in '/' since it might not always be checked
    if (containerRoot.back() != '/') {
        containerRoot += "/";
    }

    Container container(containerName,
                        containerConfig,
                        containerRoot,
                        containedCommand);

    if (!container.initialize()) {
        log_error() << "Could not setup container for preloading";
        removeDirs();
        return -1;
    }

    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create();

    { // Create a new scope so that we can do a clean up after dtors
        DBus::Glib::BusDispatcher dispatcher;
        DBus::default_dispatcher = &dispatcher;
        dispatcher.attach(ml->get_context()->gobj());
        DBus::Connection bus = DBus::Connection::SessionBus();

        /* The request_name call does not return anything but raises an
         * exception if the name cannot be requested.
         */
        std::string name = "com.pelagicore.Pelagicontain" + cookie;
        bus.request_name(name.c_str());

        /* If we can't communicate with PAM then there is nothing we can 
         * do really, better to just exit.
         */
        bool pamRunning = bus.has_name("com.pelagicore.PAM");
        if (!pamRunning) {
            log_error() << "PAM not running, exiting";
            removeDirs();
            return -1;
        }

        PAMInterface pamInterface(bus);
        ControllerInterface controllerInterface(gatewayDir);
        SystemcallInterface systemcallInterface;
        pelagicontain = new Pelagicontain(&pamInterface,
                                          ml,
                                          &controllerInterface,
                                          cookie);

        std::string objectPath = "/com/pelagicore/Pelagicontain";

        PelagicontainToDBusAdapter pcAdapter(bus, objectPath, *pelagicontain);

#ifdef ENABLE_NETWORKGATEWAY
        pelagicontain->addGateway(new NetworkGateway(controllerInterface,
                                                    systemcallInterface));
#endif

#ifdef ENABLE_PULSEGATEWAY
        pelagicontain->addGateway(new PulseGateway(gatewayDir, containerName,
                                                  controllerInterface));
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
        pelagicontain->addGateway(new DeviceNodeGateway(controllerInterface));
#endif

#ifdef ENABLE_DBUSGATEWAY
        pelagicontain->addGateway(new DBusGateway(controllerInterface,
                                                 systemcallInterface,
                                                 DBusGateway::SessionProxy,
                                                 gatewayDir,
                                                 containerName));

        pelagicontain->addGateway(new DBusGateway(controllerInterface,
                                                 systemcallInterface,
                                                 DBusGateway::SystemProxy,
                                                 gatewayDir,
                                                 containerName));
#endif

#ifdef ENABLE_CGROUPSGATEWAY
        pelagicontain->addGateway(new CgroupsGateway(controllerInterface,
                                                    systemcallInterface,
                                                    containerName));
#endif

        pcPid = pelagicontain->preload(&container);

        if (!pcPid) {
            // Fatal failure, only do necessary cleanup
            log_error() << "Could not start container, will shut down";
        } else {
            log_debug() << "Started container with PID " << pcPid;
            // setup IPC between Pelagicontain and Controller
            bool connected = pelagicontain->establishConnection();
            if (connected) {
                ml->run();
                // When we return here Pelagicontain has exited the mainloop
                log_debug("Exited mainloop");
            } else {
                // Fatal failure, only do necessary cleanup
                log_error() << "Got no connection from Controller";
                pelagicontain->shutdownContainer();
            }
        }
    }

    delete pelagicontain;
    removeDirs();

    log_debug() << "Goodbye.";
}
