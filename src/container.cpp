/*
 * Copyright (C) 2013, Pelagicore AB <tomas.hallenberg@pelagicore.com>
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

#include <fstream>
#include <unistd.h>
#include "container.h"
#include "generators.h"

using namespace std;

const char *Container::name()
{
	return m_name.c_str();
}

void Container::addGateway(Gateway *gw)
{
	m_gateways.push_back(gw);
}

int Container::run(int argc, char **argv, struct lxc_params *ct_pars)
{
	int         max_cmd_len = sysconf(_SC_ARG_MAX);
	char        lxc_command[max_cmd_len];
	char        user_command[max_cmd_len];
	int         retval = 0;
	std::string environment;

	/* Set up an environment */
	for (std::vector<Gateway *>::iterator it = m_gateways.begin();
		it != m_gateways.end(); ++it) {
		std::string env = (*it)->environment();
		if (!env.empty()) {
			environment += env + " ";
		}
	}
	debug("Using environment: %s", environment.c_str());

	/* Create container */
	sprintf (lxc_command, "DEPLOY_DIR=%s lxc-create -n %s -t pelagicontain"
                              " -f %s > /tmp/lxc_%s.log",
                              ct_pars->ct_root_dir, name(),
			      configFile(), name());
	int ret = system (lxc_command);
	if (ret) {
		log_error("%s returned %d", lxc_command, ret);
	} else {
		debug("%s returned %d", lxc_command, ret);
	}

	/* Execute command in container */
	for (int i = 2; i < argc; i++) {
		int clen = strlen (user_command);
		int nlen = strlen ((const char *) argv[i]);
		if (nlen + clen >= max_cmd_len - 256) {
			log_error ("Parameter list too long");
			exit (1);
		}
		strcat (user_command, argv[i]);
		strcat (user_command, " ");
	}

	snprintf (lxc_command, max_cmd_len, "lxc-execute -n %s -- env %s %s",
		  name(), environment.c_str(), user_command);
	ret = system (lxc_command);
	if (ret)
		log_error("%s returned %d\n", lxc_command, ret);

	/* Destroy container */
	snprintf (lxc_command, max_cmd_len, "lxc-destroy -n %s", name());
	ret = system (lxc_command);
        if (ret)
                log_error("%s returned %d", lxc_command, ret);

	return retval;
}

const char *Container::configFile()
{
	std::string path("/tmp/lxc_config_");
	path += m_name;
	return path.c_str();
}

int Container::writeConfiguration(struct lxc_params *params)
{
        debug ("Generating config to %s for IP %s",
		configFile(), params->ip_addr.c_str());

	/* Copy system config to temporary location */
	ifstream source(params->lxc_system_cfg, ios::binary);
	ofstream dest(configFile(), ios::binary);
	dest << source.rdbuf();
	source.close();

	/* Add ipv4 parameters to config */
	dest << "lxc.network.veth.pair = " << params->net_iface_name << endl;
	dest << "lxc.network.ipv4 = " << params->ip_addr << "/24" << endl;
	dest << "lxc.network.ipv4.gateway = " << params->gw_addr << endl;

	dest.close();

	return 0;
}

Container::Container(struct lxc_params *ct_pars)
{
	m_name = ct_pars->container_name;

	/* Initialize DBus socket paths */
	snprintf(ct_pars->session_proxy_socket, sizeof(ct_pars->session_proxy_socket),
		"%s/sess_%s.sock", ct_pars->ct_root_dir, name());
	snprintf(ct_pars->system_proxy_socket, sizeof(ct_pars->system_proxy_socket),
		"%s/sys_%s.sock", ct_pars->ct_root_dir, name());

	/* Initialize pulse socket paths */
	snprintf(ct_pars->pulse_socket, sizeof(ct_pars->pulse_socket),
		"%s/pulse-%s.sock", ct_pars->ct_root_dir, name());

	writeConfiguration(ct_pars);
}

Container::~Container()
{
	if (remove(configFile()) == -1) {
		log_error("Failed to remove lxc config file!");
	}
}
