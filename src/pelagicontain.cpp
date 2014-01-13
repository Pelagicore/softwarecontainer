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
#include "config.h"
#include "container.h"
#include "dbusproxy.h"
#include "debug.h"
#include "generators.h"
#include "iptables.h"
#include "pulse.h"
#include "trafficcontrol.h"

#include "CommandLineParser.h"

LOG_DEFINE_APP_IDS("PCON", "Pelagicontain");
LOG_DECLARE_CONTEXT(Pelagicontain_DefaultLogContext, "PCON", "Main context");

/*! Initialize config struct
 *
 * Initialize the config struct with various paths and parameters. Some of
 * these parameters are read som $container/config/pelagicore.conf. This
 * function verifies that all values parsed from this config file are present,
 * and returns -EINVAL if they are not.
 *
 * \param  ct_pars      Pointer to struct to initialize
 * \param  ct_base_dir  Path to container base dir
 * \param  config       Configuration parser
 * \return 0            Upon success
 * \return -EINVAL      Upon bad/missing configuration parameters
 */
static int initializeConfig(struct lxc_params *ct_pars, const char *ct_base_dir, Config *config)
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

int main (int argc, char **argv)
{
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

	if (initializeConfig(&ct_pars, argv[1], &config)) {
		log_error("Failed to initialize config. Exiting");
		return -1;
	}

	Container container(&ct_pars);

	debug("Generate iptables rules");
	IpTables rules(ct_pars.ip_addr.c_str(), config.getString("iptables-rules"));

	/* Load pulseaudio module */
	debug("Load pulseaudio module");
	Pulse pulse(ct_pars.pulse_socket);
	container.addGateway(&pulse);

	/* Limit network interface */
	debug("Limit network interface");
	limit_iface(ct_pars.net_iface_name.c_str(), ct_pars.tc_rate);

	/* Spawn proxies */
	DBusProxy sessionProxy(ct_pars.session_proxy_socket,
	                   ct_pars.main_cfg_file,
	                   DBusProxy::SessionProxy);
	
	DBusProxy systemProxy(ct_pars.system_proxy_socket,
	                  ct_pars.main_cfg_file,
	                  DBusProxy::SystemProxy);
	
	container.addGateway(&sessionProxy);
	container.addGateway(&systemProxy);

	container.run(argc, argv, &ct_pars);
}
