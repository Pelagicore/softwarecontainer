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

#include "UNIXSignalGlibHandler.h"


//LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_DEFAULT_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

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
    std::string cookie;
    const char* configFilePath = PELAGICONTAIN_DEFAULT_CONFIG;
    commandLineParser.addOption(configFilePath,
                                "with-config-file",
                                'c',
                                "Config file");

//    const char* terminalCommand = "konsole";
    const char* terminalCommand = nullptr;
    commandLineParser.addOption(terminalCommand,
                                "terminal",
                                't',
                                "Example: konsole");

    if (commandLineParser.parse(argc, argv)) {
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

    auto mainContext = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create(mainContext);

    DBus::Glib::BusDispatcher dispatcher;
    DBus::default_dispatcher = &dispatcher;

    dispatcher.attach(mainContext->gobj());

    pelagicontain::PelagicontainLib lib(mainContext, containerRoot.c_str(), cookie.c_str(), configFilePath);

    lib.getPelagicontain().getContainerState().addListener( [&] (ContainerState state) {
        if(state == ContainerState::TERMINATED) {
        	log_debug() << "Container terminated => stop main loop and quit";
        	ml->quit();
        }
    });

    if (!isError(lib.init())) {

    	lib.registerDBusService();

		// Register signalHandler with signals
		std::vector<int> signals = {SIGINT, SIGTERM};
		pelagicore::UNIXSignalGlibHandler handler(signals, [&] (int signum) {
			log_debug() << "caught signal " << signum;
			switch(signum) {
			case SIGCHLD:
				break;
			default:
				lib.shutdown();
				break;
			}
		}, ml->get_context()->gobj());

		if (terminalCommand != nullptr) {
			std::string s = pelagicore::formatString("%s -e lxc-attach -n %s", terminalCommand, lib.getContainer().name());
			system(s.c_str());
		}

		ml->run();

		log_debug() << "Goodbye.";
    }
    else
    	log_error( ) << "Could not initialize pelagicontain";
}
