/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <cstdlib>

#include "config.h"
#include "container.h"
#include "dbusproxy.h"
#include "debug.h"
#include "generators.h"
#include "iptables.h"
#include "pulse.h"
#include "trafficcontrol.h"
#include "pelagicontain.h"


Pelagicontain::Pelagicontain(PAMAbstractInterface *pamInterface):
	m_pamInterface(pamInterface)
{
}

Pelagicontain::~Pelagicontain()
{
}

using namespace pelagicore;

/*! Initialize config struct
 *
 * Initialize the config struct with various paths and parameters. Some of
 * these parameters are read from $container/config/pelagicore.conf. This
 * function verifies that all values parsed from this config file are present,
 * and returns -EINVAL if they are not.
 *
 * \param  ct_pars      Pointer to struct to initialize
 * \param  ct_base_dir  Path to container base dir
 * \param  config       Configuration parser
 * \return 0            Upon success
 * \return -EINVAL      Upon bad/missing configuration parameters
 */
int Pelagicontain::initializeConfig(struct lxc_params *ct_pars, const char *ct_base_dir, Config *config)
{
	char *ip_addr_net;

	/* Initialize */
	ct_pars->container_name = gen_ct_name();

	snprintf(ct_pars->ct_conf_dir,
		sizeof(ct_pars->ct_conf_dir),
		"%s/config/",
		ct_base_dir);
	
	snprintf(ct_pars->ct_root_dir,
		sizeof(ct_pars->ct_root_dir),
		 "%s/rootfs/",
		ct_base_dir);

	snprintf(ct_pars->main_cfg_file,
		  sizeof(ct_pars->main_cfg_file),
		  "%s/pelagicontain.conf",
	          ct_pars->ct_conf_dir);

	config->read(ct_pars->main_cfg_file);

	ct_pars->lxc_system_cfg = config->getString("lxc-config-template");
	if (!ct_pars->lxc_system_cfg) {
		log_error("Unable to read lxc-config-template from config!");
		return -EINVAL;
	}

	ct_pars->tc_rate = config->getString("bandwidth-limit");
	if (!ct_pars->tc_rate) {
		log_error("Unable to read bandwidth-limit from config!");
		return -EINVAL;
	}

	ip_addr_net = config->getString("ip-addr-net");
	if (!ip_addr_net) {
		log_error("Unable to read ip-addr-net from config!");
		return -EINVAL;
	}

	ct_pars->gw_addr = config->getString("gw-ip-addr");
	if (!ct_pars->gw_addr) {
		log_error("Unable to read gw-ip-addr from config!");
		return -EINVAL;
	}

	ct_pars->ip_addr = gen_ip_addr(ip_addr_net);
	ct_pars->net_iface_name = gen_net_iface_name(ip_addr_net);

	return 0;
}

/*! Initialize the Pelagicpontain object before usage */
int Pelagicontain::initialize(struct lxc_params &ct_pars, Config &config)
{
	m_container = Container(&ct_pars);

// 	debug("Generate iptables rules");
// 	IpTables rules(ct_pars.ip_addr.c_str(), config.getString("iptables-rules"));
//
// 	// Load pulseaudio module
// 	debug("Load pulseaudio module");
// 	Pulse pulse(ct_pars.pulse_socket);
// 	m_container.addGateway(&pulse);
//
// 	// Limit network interface
// 	debug("Limit network interface");
// 	limit_iface(ct_pars.net_iface_name.c_str(), ct_pars.tc_rate);
//
// 	Spawn proxies
	m_gateways.push_back(new DBusProxy(ct_pars.session_proxy_socket,
		ct_pars.main_cfg_file, DBusProxy::SessionProxy));

	m_gateways.push_back(new DBusProxy(ct_pars.system_proxy_socket,
		ct_pars.main_cfg_file, DBusProxy::SystemProxy));

	return 0;
}

// Launch the container. This is a non-blocking operation
pid_t Pelagicontain::run(int numParameters, char **parameters, struct lxc_params *ct_pars,
	const std::string &cookie)
{
	m_cookie = cookie;

	// Get the commands to run in a separate process
	std::vector<std::string> commands;
	commands = m_container.commands(numParameters, parameters, ct_pars, m_gateways);

	std::string createCommand = commands[0];
	std::string executeCommand = commands[1];
	std::string destroyCommand = commands[2];

	log_debug(createCommand.c_str());
	system(createCommand.c_str());

	pid_t pid = fork();
	if (pid == 0) { //child
		/**
		 * lxc-execute inherits file descriptors from parent which seems to cause
		 * a crash, so we close a bunch to avoid that, fd 0, 1, and 2 are
		 * kept because they are standard fd's that we want (e.g. see output
		 * on stdout). (the number 30 is arbitrary)
		 */
		for (int i = 3; i < 30; i++)
			close(i);

		//TODO: Is there any way to get the pid of the Controller so we
		// can use that to tell it to shut down nicely. Currently we can only
		// tell lxc-execute to shut down but then we don't know if Controller
		// was actually shut down properly.
		log_debug(executeCommand.c_str());
		system(executeCommand.c_str());

		log_debug(destroyCommand.c_str());
		system(destroyCommand.c_str());
		exit(0);
	} // Parent

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

	// TODO: Should we check if gateways have been activated already?
	activateGateways();

	m_controller.startApp();
}

void Pelagicontain::setGatewayConfigs(const std::map<std::string, std::string> &configs)
{
	// Go through the received configs and see if they match any of
	// the running gateways, if so: set their respective config
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
	// Tell Controller to shut down the app
	// Controller will exit when the app has shut down and then
	// lxc-execute will return and lxc-destroy be run (see above
	// code in the forked child)
	m_controller.shutdown();

	// Shut down (clean up) all Gateways
	shutdownGateways();

	m_pamInterface->unregisterClient(m_appId);

	// exit Pelagicontain
	// TODO: Is there a problem with exiting here without konowing if
	// Controller has exited?
	int status = 0;
	wait(&status);
	log_debug("Wait status: %d", status);
	if (WIFEXITED(status))
		log_debug("#### child exited");
	if (WIFSIGNALED(status))
		log_debug("#### child exited by signal: %d", WTERMSIG(status));
	raise(SIGINT);
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
