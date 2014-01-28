/*
 * Copyright (C) 2013, Pelagicore AB <jonatan.palsson@pelagicore.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
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

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

Pelagicontain::Pelagicontain(const PAMInterface &pamInterface):
	m_pamInterface(pamInterface)
{
	pipe(m_fd);
}

Pelagicontain::~Pelagicontain()
{
}

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
// 	DBusProxy sessionProxy(ct_pars.session_proxy_socket,
// 		ct_pars.main_cfg_file, DBusProxy::SessionProxy);

// 	DBusProxy systemProxy(ct_pars.system_proxy_socket,
// 		ct_pars.main_cfg_file, DBusProxy::SystemProxy);

// 	m_container.addGateway(&sessionProxy);
// 	m_container.addGateway(&systemProxy);

	return 0;
}

/*! Launch the container. This is a non-blocking operation */
pid_t Pelagicontain::run(int numParameters, char **parameters, struct lxc_params *ct_pars)
{
	// Get the commands to run in a separate process
	std::vector<std::string> commands;
	commands = m_container.commands(numParameters, parameters, ct_pars);

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
	m_pamInterface.RegisterClient("cookie" /* This comes from the launcher*/, appId);
}

//TODO: Put all the below Controller stuff behind a class interface

void Pelagicontain::update(const std::vector<std::string> &config)
{
// 	log_debug("######### PAM called Update");
	// TODO: Set configurations on all gateways we got a config for

	// Call PAM::UpdateFinished
	m_pamInterface.UpdateFinished();

	// TODO: Activate all gateways, when all have responded:

	// Run app inside container
	int fd = open("/tmp/test/rootfs/in_fifo", O_WRONLY);
	write(fd, "1\n", 2);
}

bool Pelagicontain::shutdown()
{
	// Tell Controller to shut down the app
	int fd = open("/tmp/test/rootfs/in_fifo", O_WRONLY);
	write(fd, "2\n", 2);

	// When controller is finished:
	// Shutdown Controller
	// NOTE: There seems to be a problem here, we tell Controller to shut down
	// the App and then go ahead and kill Controller. Should we wait for the
	// Controller to confirm the App is shut down?
// 	write(fd, "3\n", 2);

	// Shut down (clean up) all Gateways
	// When all Gateways are finished:

	// Call PAM::UnregisterClient(appId)
	//TODO: The real appId should be used!
	m_pamInterface.UnregisterClient("the-app-ID");

	// Shut down LXC and exit Pelagicontain
	// NOTE: Actually lxc-destroy is run as soon as Controller shuts down as
	// the call is placed in the fork after lxc-execute returns... a better
	// question is how we should return a value from here and still exit here
// 	exit(0);
	return true;
}
