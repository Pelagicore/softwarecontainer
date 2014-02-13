/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTAINER_H
#define CONTAINER_H

#include <string>
#include <vector>

#include "gateway.h"

/*! Container is an abstraction of the specific containment technology used.
 *
 * The Container class is meant to be an abstraction of the containment
 * technology, so that Pelagicontain can view it as a generic concept that
 * implements the specifics behind the conceptual phases of 'Pereload', 'Launch',
 * and 'Shutdown'.
 */
class Container
{
public:
	Container();
	Container(const std::string &name, const std::string &configFile);
	~Container();

	const char *name();
	std::vector<std::string> commands(const std::string &containedCommand,
		const std::vector<Gateway *> &gateways,
		const std::string &appRoot);

	std::string m_name;
    std::string m_configFile;
};

#endif //CONTAINER_H
