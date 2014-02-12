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

Container::Container(const std::string &name, const std::string &configFile) :
    m_name(name), m_configFile(configFile) {}

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
		m_configFile.c_str(),
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
