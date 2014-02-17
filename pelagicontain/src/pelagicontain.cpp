/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <sys/wait.h>
#include <unistd.h>

#include "container.h"
#include "debug.h"
#include "pelagicontain.h"

Pelagicontain::Pelagicontain(PAMAbstractInterface *pamInterface,
	MainloopAbstractInterface *mainloopInterface):
	m_pamInterface(pamInterface), m_mainloopInterface(mainloopInterface)
{
}

Pelagicontain::~Pelagicontain()
{
}

/* Initialize the Pelagicpontain object before usage */
int Pelagicontain::initialize(const std::vector<Gateway *> &gateways,
	const std::string &containerName,
	const std::string &containerConfig)
{
	m_gateways = gateways;
	m_container = Container(containerName, containerConfig);

	return 0;
}

/* Launch the container. This is a non-blocking operation */
pid_t Pelagicontain::run(const std::string &containerRoot,
	const std::string &containedCommand,
	const std::string &cookie)
{
	m_cookie = cookie;

	/* Get the commands to run in a separate process */
	std::vector<std::string> commands;
	std::string appRoot = containerRoot + "/rootfs/";
	commands = m_container.commands(containedCommand, m_gateways, appRoot);

	std::string createCommand = commands[0];
	std::string executeCommand = commands[1];
	std::string destroyCommand = commands[2];

	/* Calling create and execute is part of the Preloading phase.
	 * When the call to execute returns, it means the shutdown phase
	 * has been initiated and destroy is a part of that phase (destroy
	 * is called when execute has returned).
	 *
	 * Calls to Controller to e.g. launch the contained app, which occurs
	 * between preload and shutdown, is part of the launch phase.
	 */
	log_debug(createCommand.c_str());
	system(createCommand.c_str());

	pid_t pid = fork();
	if (pid == 0) { /* child */
		/* NOTE: lxc-execute inherits file descriptors from parent which seems
		 * to cause a crash, so we close a bunch to avoid that, fd 0, 1, and 2
		 * are kept because they are standard fd's that we want (e.g. see output
		 * on stdout). (the number 30 is arbitrary)
		 */
		for (int i = 3; i < 30; i++)
			close(i);

		log_debug(executeCommand.c_str());
		system(executeCommand.c_str());

		log_debug(destroyCommand.c_str());
		system(destroyCommand.c_str());
		exit(0);
	} /* Parent */

	return pid;
}

void Pelagicontain::launch(const std::string &appId) {
	m_appId = appId;
	m_pamInterface->registerClient(m_cookie, m_appId);
}

void Pelagicontain::update(const std::map<std::string, std::string> &configs)
{
	setGatewayConfigs(configs);

	m_pamInterface->updateFinished(m_appId);

	activateGateways();

	m_controller.startApp();
}

void Pelagicontain::setGatewayConfigs(const std::map<std::string, std::string> &configs)
{
	/* Go through the received configs and see if they match any of
	 * the running gateways, if so: set their respective config
	 */
	std::string config;
	std::string gatewayId;

	for (std::vector<Gateway *>::iterator gateway = m_gateways.begin();
		gateway != m_gateways.end(); ++gateway) {
		gatewayId = (*gateway)->id();
		if (configs.count(gatewayId) != 0) {
			config = configs.at(gatewayId);
			(*gateway)->setConfig(config);
		}
	}
}

void Pelagicontain::activateGateways()
{
	for (std::vector<Gateway *>::iterator gateway = m_gateways.begin();
		gateway != m_gateways.end(); ++gateway) {
		(*gateway)->activate();
	}
}

void Pelagicontain::shutdown()
{
	/* Tell Controller to shut down the app
	 * Controller will exit when the app has shut down and then
	 * lxc-execute will return and lxc-destroy be run (see above
	 * code in the forked child)
	 */
	m_controller.shutdown();

	/* Shut down (clean up) all Gateways */
	shutdownGateways();

	m_pamInterface->unregisterClient(m_appId);

	/* Wait for Controller */
	int status = 0;
	wait(&status);
	if (WIFEXITED(status))
		log_debug("Child exited");
	if (WIFSIGNALED(status))
		log_debug("Child exited by signal: %d", WTERMSIG(status));

	m_mainloopInterface->leave();
}

void Pelagicontain::shutdownGateways()
{
	for (std::vector<Gateway *>::iterator gateway = m_gateways.begin();
		gateway != m_gateways.end(); ++gateway) {
		if (!(*gateway)->teardown())
			log_warning("Could not teardown gateway cleanly");
		delete (*gateway);
	}
}
