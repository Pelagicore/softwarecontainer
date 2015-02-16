/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "pelagicontain-lib.h"
#include <glibmm.h>
#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

#include <signal.h>
#include <unistd.h>
#include "CommandLineParser.h"
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

#include "UNIXSignalGlibHandler.h"

#include "pelagicore-DBusCpp.h"

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_DEFAULT_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

/**
 * Remove the dirs created by main when we cleanup
 */
void removeDirs(const std::string& containerDir)
{
    std::string gatewayDir = containerDir + "/gateways";
    if (rmdir(gatewayDir.c_str()) == -1) {
        log_error() << "Cannot delete dir " << gatewayDir << " , " << strerror(errno);
    }

    if (rmdir(containerDir.c_str()) == -1) {
        log_error() << "Cannot delete dir " << containerDir << " , " << strerror(errno);
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

int main(int argc, char * *argv)
{
    const char *summary = "Pelagicore container utility. "
            "Requires an absolute path to the container root, "
            "the command to run inside the container and "
            "an alphanumerical cookie string as first, second and"
            "third argument respectively";
    const char *paramsDescription = "[container root directory (abs path)] [cookie]";

    pelagicore::CommandLineParser commandLineParser(summary,
            paramsDescription,
            PACKAGE_VERSION,
            "");

    std::string containerRoot;
    std::string cookie;
    const char *configFilePath = PELAGICONTAIN_DEFAULT_CONFIG;
    commandLineParser.addOption(configFilePath,
            "with-config-file",
            'c',
            "Config file");

    //    const char* terminalCommand = "konsole -e";
    const char *terminalCommand = "";
    commandLineParser.addOption(terminalCommand,
            "terminal",
            't',
            "Example: konsole -e");

    if ( commandLineParser.parse(argc, argv) ) {
        return -1;
    }

    if (argc < 3) {
        log_error() << "Invalid arguments";
        commandLineParser.printHelp();
        return -1;
    } else {
        containerRoot = std::string(argv[1]);
        cookie = std::string(argv[2]);
    }

    if (containerRoot.c_str()[0] != '/') {
        log_error() << "Path to container root must be absolute";
        commandLineParser.printHelp();
        return -1;
    }

    std::string containerName = Generator::gen_ct_name();
    std::string containerConfig(configFilePath);

    // Create gateway directory for container in containerRoot/gateways.
    // This dir will contain various sockets etc acting as gateways
    // in/out of the container.
    std::string containerDir = containerRoot + "/" + containerName;
    std::string gatewayDir = containerDir + "/gateways";

    Container container(containerName,
                        containerConfig,
                        containerRoot);

    if (isError(container.initialize())) {
        log_error() << "Could not setup container for preloading";
        removeDirs(containerDir);
        return -1;
    }

    auto mainContext = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create(mainContext);

    DBus::Glib::BusDispatcher dispatcher;
    DBus::default_dispatcher = &dispatcher;
    dispatcher.attach( mainContext->gobj() );

    pelagicontain::PelagicontainLib lib(containerRoot.c_str(), configFilePath);
    lib.setMainLoopContext(mainContext);

    //    std::string busName = "com.pelagicore.Pelagicontain" + cookie;
    //    pelagicore::GLibDBusCppFactory dbusGlibFactory;
    //    dbusGlibFactory.getConnection().request_name( busName.c_str() );
    //    dbusGlibFactory.registerAdapter<PelagicontainToDBusAdapter>( "/com/pelagicore/Pelagicontain", lib.getPelagicontain() );

#ifdef ENABLE_NETWORKGATEWAY
        NetworkGateway networkGateway(controllerInterface, systemcallInterface);
        pelagicontain.addGateway(networkGateway);
#endif

#ifdef ENABLE_PULSEGATEWAY
        PulseGateway pulseGateway(gatewayDir, containerName, controllerInterface);
        pelagicontain.addGateway(pulseGateway);
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
        DeviceNodeGateway deviceNodeGateway(controllerInterface);
        pelagicontain.addGateway(deviceNodeGateway);
#endif

#ifdef ENABLE_DBUSGATEWAY
        DBusGateway sessionBusGateway(controllerInterface,
                                                         systemcallInterface,
                                                         DBusGateway::SessionProxy,
                                                         gatewayDir,
                                                         containerName);

        pelagicontain.addGateway(sessionBusGateway);

        DBusGateway systemBusGateway(controllerInterface,
                                                         systemcallInterface,
                                                         DBusGateway::SystemProxy,
                                                         gatewayDir,
                                                         containerName);
        pelagicontain.addGateway(systemBusGateway);
#endif

#ifdef ENABLE_CGROUPSGATEWAY
        pelagicontain->addGateway(new CgroupsGateway(controllerInterface,
                                                    systemcallInterface,
                                                    containerName));
#endif

        pid_t pcPid = pelagicontain.preload(&container);

		if (!pcPid) {
			// Fatal failure, only do necessary cleanup
			log_error() << "Could not start container, will shut down";
		} else {
			log_debug() << "Started container with PID " << pcPid;
			// setup IPC between Pelagicontain and Controller
			if (!isError(pelagicontain.establishConnection())) {

				if (terminalCommand != nullptr) {
					std::string s = pelagicore::formatString("%s -e lxc-attach -n %s", terminalCommand, container.name());
					system(s.c_str());
				}

				ml->run();
				// When we return here Pelagicontain has exited the mainloop
				log_debug() << "Exited mainloop";
			} else {
				// Fatal failure, only do necessary cleanup
				log_error() << "Got no connection from Controller";
				pelagicontain.shutdownContainer();
			}
		}
    }
    lib.getPelagicontain().getContainerState().addListener([&] (ContainerState state) {
                if (state == ContainerState::TERMINATED) {
                    log_debug() <<
                    "Container terminated => stop main loop and quit";
                    ml->quit();
                }
            });

    if ( !isError( lib.init() ) ) {

        // Register signalHandler with signals
        std::vector<int> signals = {SIGINT, SIGTERM};
        pelagicore::UNIXSignalGlibHandler handler( signals, [&] (int signum) {
                    log_debug() << "caught signal " << signum;
                    switch (signum) {
                    case SIGCHLD :
                        break;
                    default :
                        lib.shutdown();
                        break;
                    }
                }, ml->get_context()->gobj() );

        if ( (terminalCommand != nullptr) && (strlen(terminalCommand) != 0) ) {
            lib.openTerminal(terminalCommand);
        }

        log_debug() << "Entering main loop";
        ml->run();
        log_debug() << "Main loop exited";
    } else {
        log_error() << "Could not initialize pelagicontain";
    }
}
