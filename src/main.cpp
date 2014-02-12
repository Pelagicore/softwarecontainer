/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <signal.h>

#include <iostream>

#include "CommandLineParser.h"

#include "debug.h"
#include "paminterface.h"
#include "pelagicontain.h"
#include "pelagicontaintodbusadapter.h"

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

void myHandler(int s){
	log_debug("Caught signal %d", s);
	exit(0);
}

int main(int argc, char **argv)
{
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = myHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	CommandLineParser commandLineParser("Pelagicore container utility\n",
		"[deploy directory (abs path)] [command]",
		PACKAGE_VERSION,
		"This tool ......");

	int myOptionValue = 0;
	commandLineParser.addArgument(myOptionValue, "myoption", 'o', "An option");

	if (commandLineParser.parse(argc, argv))
		return -1;

	/* argv[1] = container root directory
	 * argv[2] = command to run inside container
	 * argv[3] = cookie to append to object path
	 */
	if (argc < 3 || argv[1][0] != '/') {
		log_error("Invalid arguments");
		commandLineParser.printHelp();
		return -1;
	}

	DBus::BusDispatcher dispatcher;
	DBus::default_dispatcher = &dispatcher;
	DBus::Connection bus = DBus::Connection::SessionBus();

	bus.request_name("com.pelagicore.Pelagicontain");

	PAMInterface pamInterface(bus);
	Pelagicontain pelagicontain(&pamInterface);

	std::string cookie(argv[3]);
	std::string baseObjPath("/com/pelagicore/Pelagicontain/");
	std::string fullObjPath = baseObjPath + cookie;

	PelagicontainToDBusAdapter pcAdapter(bus, fullObjPath, pelagicontain);

	std::string containerRoot(argv[1]);

	pelagicontain.initialize(containerRoot);
	std::string containedCommand(argv[2]);
	pid_t pcPid = pelagicontain.run(containedCommand, cookie);
	log_debug("Started Pelagicontain with PID: %d", pcPid);
	dispatcher.enter();
}
