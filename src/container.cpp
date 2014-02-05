/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <fstream>
#include <unistd.h>
#include "container.h"
#include "generators.h"

using namespace std;

Container::Container ()
{

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
// 	if (remove(configFile()) == -1)
// 		log_error("Failed to remove lxc config file!");
}

const char *Container::name()
{
	return m_name.c_str();
}

std::vector<std::string> Container::commands(int numParams, char **params,
	struct lxc_params *ct_pars, const std::vector<Gateway *> &gateways)
{
	int max_cmd_len = sysconf(_SC_ARG_MAX);
	char lxc_command[max_cmd_len];
	char user_command[max_cmd_len];
	std::string environment;
	std::vector<std::string> commands;

	// Set up an environment
	for (std::vector<Gateway *>::const_iterator it = gateways.begin();
		it != gateways.end(); ++it) {
		std::string env = (*it)->environment();
		if (!env.empty())
			environment += env + " ";
	}
	log_debug("Using environment: %s", environment.c_str());

	// Command to create container
	sprintf(lxc_command, "DEPLOY_DIR=%s lxc-create -n %s -t pelagicontain"
				" -f %s > /tmp/lxc_%s.log",
				ct_pars->ct_root_dir, name(),
				configFile(), name());
	commands.push_back(std::string(lxc_command));

	// Create command to execute inside container
	// The last parameter is the cookie and should not be used here
	for (int i = 2; i < numParams-1; i++) {
		int clen = strlen(user_command);
		int nlen = strlen((const char *)params[i]);
		if (nlen + clen >= max_cmd_len - 256) {
			log_error("Parameter list too long");
			exit(1);
		}
		strcat(user_command, params[i]);
		strcat(user_command, " ");
	}

	// Command to execute inside container
	snprintf(lxc_command, max_cmd_len, "lxc-execute -n %s -- env %s %s",
		name(), environment.c_str(), user_command);
	commands.push_back(std::string(lxc_command));

	// Command to destroy container
	snprintf(lxc_command, max_cmd_len, "lxc-destroy -n %s", name());
	commands.push_back(std::string(lxc_command));

	return commands;
}

const char *Container::configFile()
{
	std::string path("/tmp/lxc_config_");
	path += m_name;
	return path.c_str();
}

int Container::writeConfiguration(struct lxc_params *params)
{
        debug("Generating config to %s for IP %s", configFile(), params->ip_addr.c_str());

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
