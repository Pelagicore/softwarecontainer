#include "CommandLineParser.h"

#include "pelagicontain.h"
#include "pelagicontaincommon.h"
#include "pelagicontaintodbusadapter.h"

int main (int argc, char **argv)
{
	DBus::BusDispatcher dispatcher;
	Pelagicontain pelagicontain;

	DBus::default_dispatcher = &dispatcher;
	DBus::Connection bus = DBus::Connection::SessionBus();
	PelagicontainToDBusAdapter pcAdapter (bus, pelagicontain);

	bus.request_name("com.pelagicore.Pelagicontain");


	CommandLineParser commandLineParser("Pelagicore container utility\n",
		"[deploy directory (abs path)] [command]",
		PACKAGE_VERSION,
		"This tool ......");

	int myOptionValue = 0;
	commandLineParser.addArgument(myOptionValue, "myoption", 'o', "An option");

	if (commandLineParser.parse(argc, argv))
		exit(-1);

	struct lxc_params ct_pars;

	if (argc < 3 || argv[1][0] != '/') {
		log_error("Invalid arguments");
		commandLineParser.printHelp();
		return -1;
	}

	Config config;

	if (Pelagicontain::initializeConfig(&ct_pars, argv[1], &config)) {
		log_error("Failed to initialize config. Exiting");
		return -1;
	}

	pelagicontain.initialize (ct_pars, config);
	log_debug("Started Pelagicontain with PID: %d", pelagicontain.run (argc, argv, &ct_pars));
	dispatcher.enter();
}
