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
	MainloopAbstractInterface *mainloopInterface,
	ControllerAbstractInterface *controllerInterface):
	m_container(NULL),
	m_pamInterface(pamInterface),
	m_mainloopInterface(mainloopInterface),
	m_controllerInterface(controllerInterface)
{
}

Pelagicontain::~Pelagicontain()
{
	if (m_container)
	{
		delete m_container;
	}
}

/* Initialize the Pelagicontain object before usage */
int Pelagicontain::initialize(const std::string &containerName,
	const std::string &containerConfig, const std::string &containerRoot)
{
	m_container = new Container(containerName, containerConfig, containerRoot);

	return 0;
}

void Pelagicontain::addGateway(Gateway *gateway)
{
	m_gateways.push_back(gateway);
}

/* Preload the container. This is a non-blocking operation */
pid_t Pelagicontain::preload(const std::string &containerRoot,
	const std::string &containedCommand,
	const std::string &cookie)
{
	if (!m_container)
	{
		log_error("initialize() has not been called prior to preload.");
	}
	m_cookie = cookie;

	/* Get the commands to run in a separate process */
	std::vector<std::string> commands;
	commands = m_container->commands(containedCommand, m_gateways);

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

                /* This will not return until the Controller (or whatever is 
                 * executed in the container) exits.
                 */
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
	if (m_container)
	{
		// this should always be true except when unit-testing.
		m_container->setApplication(appId);
	}
	m_pamInterface->registerClient(m_cookie, m_appId);
}

void Pelagicontain::update(const std::map<std::string, std::string> &configs)
{
	setGatewayConfigs(configs);

	m_pamInterface->updateFinished(m_appId);

	activateGateways();

	m_controllerInterface->startApp();
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
	m_controllerInterface->shutdown();

	/* Wait for Controller to finish and exit */
	int status = 0;
	wait(&status);
	if (WIFEXITED(status))
		log_debug("Child exited");
	if (WIFSIGNALED(status))
		log_debug("Child exited by signal: %d", WTERMSIG(status));

        /* Shut down (clean up) all Gateways */
        shutdownGateways();

        m_pamInterface->unregisterClient(m_appId);

	m_mainloopInterface->leave();
}

void Pelagicontain::shutdownGateways()
{
	for (std::vector<Gateway *>::iterator gateway = m_gateways.begin();
		gateway != m_gateways.end(); ++gateway)
	{
		if (!(*gateway)->teardown())
			log_warning("Could not teardown gateway cleanly");
		delete (*gateway);
	}

	m_gateways.clear();
}
