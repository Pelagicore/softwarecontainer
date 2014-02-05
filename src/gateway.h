/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef GATEWAY_H
#define GATEWAY_H

#include <string>
#include <iostream>
#include "debug.h"

/*! \brief Gateway base class
 *  \file gateway.h
 *
 * Gateway abstract base class for Pelagicontain
 */
class Gateway
{
public:
	virtual ~Gateway() {};

	virtual std::string environment() = 0;
	virtual std::string id() = 0;
	virtual bool setConfig(const std::string &config) = 0;
	virtual bool activate() = 0;
	virtual bool teardown() {return true;}
};

#endif /* GATEWAY_H */
