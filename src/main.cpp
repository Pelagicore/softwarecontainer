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

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

#ifndef CONFIG
    #error Must define CONFIG; path to configuration file (/etc/pelagicontain?)
#endif

int main(int argc, char **argv)
{
	std::string containerConfig(CONFIG);

	CommandLineParser commandLineParser("Pelagicore container utility\n",
		"[deploy directory (abs path)] [command] [cookie]",
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
	if (argc < 4 || argv[1][0] != '/') {
		log_error("Invalid arguments");
		commandLineParser.printHelp();
		return -1;
	}

	DBus::BusDispatcher dispatcher;
	DBus::default_dispatcher = &dispatcher;
	DBus::Connection bus = DBus::Connection::SessionBus();

	/* Pelagicontain needs an interface to the mainloop so it can
	 * exit it when we are to clean up and shut down
	 */
	DBusMainloop dbusmainloop(&dispatcher);

	bus.request_name("com.pelagicore.Pelagicontain");

	PAMInterface pamInterface(bus);
	Pelagicontain pelagicontain(&pamInterface, &dbusmainloop);

	std::string cookie(argv[3]);
	std::string baseObjPath("/com/pelagicore/Pelagicontain/");
	std::string fullObjPath = baseObjPath + cookie;

	PelagicontainToDBusAdapter pcAdapter(bus, fullObjPath, pelagicontain);

	std::string containerRoot(argv[1]);
	std::string containerName = gen_ct_name();

	std::vector<Gateway *> gateways;

	gateways.push_back(new NetworkGateway);

	gateways.push_back(new PulseGateway(containerRoot, containerName));

	gateways.push_back(new DBusGateway(DBusGateway::SessionProxy,
		containerRoot, containerName));

	gateways.push_back(new DBusGateway(DBusGateway::SystemProxy,
		containerRoot, containerName));

	pelagicontain.initialize(gateways, containerName, containerConfig);

	std::string containedCommand(argv[2]);
	pid_t pcPid = pelagicontain.run(containerRoot, containedCommand, cookie);

	log_debug("Started Pelagicontain with PID: %d", pcPid);

	dbusmainloop.enter();
}
