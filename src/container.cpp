/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <fstream>
#include <unistd.h>
#include "container.h"
#include "debug.h"

using namespace std;

Container::Container ()
{

}

Container::Container(const std::string &name)
{
	m_name = name;

	writeConfiguration();
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

std::vector<std::string> Container::commands(const std::string &containedCommand,
	const std::vector<Gateway *> &gateways,
	const std::string &appRoot)
{
	int max_cmd_len = sysconf(_SC_ARG_MAX);
	char lxc_command[max_cmd_len];
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
	sprintf(lxc_command,
		"DEPLOY_DIR=%s lxc-create -n %s -t pelagicontain"
		" -f %s > /tmp/lxc_%s.log",
		appRoot.c_str(),
		name(),
		configFile(),
		name());
	commands.push_back(std::string(lxc_command));

	// Create command to execute inside container
	snprintf(lxc_command, max_cmd_len, "lxc-execute -n %s -- env %s %s",
		name(), environment.c_str(), containedCommand.c_str());
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

int Container::writeConfiguration()
{
	/* NOTE: Previously there were additions written to the config file
	 * here. Currently the system config is just copied and not ameneded
	 * with more information, which means this flow is pretty useless at the
	 * moment. If there's no use for this option to amend the config, this
	 * code can be removed and the rest of the flow cleaned up so the
	 * "lxc_config_<container-name>" is not used at all. In that case the
	 * "system config" copied below could be used as it is.
	 */
        log_debug("Generating config to %s", configFile());

	/* Copy system config to temporary location */
	ifstream source("/etc/pelagicontain", ios::binary);
	ofstream dest(configFile(), ios::binary);
	dest << source.rdbuf();
	source.close();
	dest.close();

	return 0;
}
