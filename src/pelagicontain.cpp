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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>


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
	pipe (m_fd);
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
// 	//Spawn proxies
// 	DBusProxy sessionProxy(ct_pars.session_proxy_socket,
// 		ct_pars.main_cfg_file, DBusProxy::SessionProxy);
// 	
// 	DBusProxy systemProxy(ct_pars.system_proxy_socket,
// 		ct_pars.main_cfg_file, DBusProxy::SystemProxy);
// 	
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

		// Run all commands from the Container in the child process
		for (std::vector<std::string>::iterator it = commands.begin();
			it != commands.end(); ++it) {
			const char *command = (*it).c_str();
			int ret = system(command);
			if (ret)
				log_error("%s returned %d", command, ret);
			else
				log_debug("%s returned %d", command, ret);
		}
		exit(0);
	} // Parent

	return pid;
}

void Pelagicontain::launch(const std::string &appId) {
	m_pamInterface.RegisterClient("cookie" /* This comes from the launcher*/, appId);
	int fd = open("/tmp/test/rootfs/in_fifo", O_WRONLY);
	std::cout << "--->" << write(fd, "abc\r\n", 5) << std::endl;
}

void Pelagicontain::update(const std::vector<std::string> &config)
{
	log_debug("######### PAM called Update");
	// Set configurations on all gateways we got a config for
	// Call PAM::UpdateFinished
	// Activate all gateways, when all have responded:
	// Run app inside container
}
