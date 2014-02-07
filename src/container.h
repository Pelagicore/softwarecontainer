/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTAINER_H
#define CONTAINER_H

#include <string>
#include <vector>
#include "pelagicontaincommon.h"
#include "config.h"
#include "gateway.h"

/*! \brief Container base class
 *  \file container.h
 *
 *  Container base class for Pelagicontain
 */
class Container
{
public:
	Container ();
	Container(struct lxc_params *ct_pars);
	~Container();

	const char *name();
	std::vector<std::string> commands(int numParams, char **params,
		struct lxc_params *ct_pars, const std::vector<Gateway *> &gateways);

private:
	const char *configFile();
	int writeConfiguration(struct lxc_params *params);

	std::string m_name;
};

#endif //CONTAINER_H
