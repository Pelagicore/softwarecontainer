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
	const char *summary = "Pelagicore container utility. "
		"Requires an absolute path to the container root, "
		"the command to run inside the container and "
		"an alphanumerical cookie string as first, second and third "
		"argument respectively";
	const char *paramsDescription = "[container root directory (abs path)] "
		"[command] [cookie]";

	CommandLineParser commandLineParser(summary,
		paramsDescription,
		PACKAGE_VERSION,
		"");

	if (commandLineParser.parse(argc, argv))
		return -1;

	std::string containerRoot;
	std::string containedCommand;
	std::string cookie;
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

	DBus::BusDispatcher dispatcher;
	DBus::default_dispatcher = &dispatcher;
	DBus::Connection bus = DBus::Connection::SessionBus();

	/* Pelagicontain needs an interface to the mainloop so it can
	 * exit it when we are to clean up and shut down
	 */
	DBusMainloop dbusmainloop(&dispatcher);

	/* The request_name call does not return anything but raises an
	 * exception if the name cannot be requested.
	 */
	bus.request_name("com.pelagicore.Pelagicontain");

	PAMInterface pamInterface(bus);
	ControllerInterface controllerInterface(containerRoot);
	Pelagicontain pelagicontain(&pamInterface, &dbusmainloop, &controllerInterface);

	std::string baseObjPath("/com/pelagicore/Pelagicontain/");
	std::string fullObjPath = baseObjPath + cookie;

	PelagicontainToDBusAdapter pcAdapter(bus, fullObjPath, pelagicontain);

	std::string containerName = gen_ct_name();
	std::string containerConfig(CONFIG);

	std::vector<Gateway *> gateways;

	gateways.push_back(new NetworkGateway);

	gateways.push_back(new PulseGateway(containerRoot, containerName));

	gateways.push_back(new DBusGateway(DBusGateway::SessionProxy,
		containerRoot, containerName, containerConfig));

	gateways.push_back(new DBusGateway(DBusGateway::SystemProxy,
		containerRoot, containerName, containerConfig));

	pelagicontain.initialize(gateways, containerName, containerConfig);

	pid_t pcPid = pelagicontain.preload(containerRoot, containedCommand, cookie);

	log_debug("Started Pelagicontain with PID: %d", pcPid);

	dbusmainloop.enter();
}
