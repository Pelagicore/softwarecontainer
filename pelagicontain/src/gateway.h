/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef GATEWAY_H
#define GATEWAY_H

#include <string>
#include "controllerinterface.h"

/*! Gateway base class
 *
 * Gateway base class for Pelagicontain
 */
class Gateway
{
public:

	Gateway(ControllerAbstractInterface *controllerInterface):
		m_controllerInterface(controllerInterface){};
	Gateway() {};
	virtual ~Gateway() {};

	virtual std::string environment() = 0;
	virtual std::string id() = 0;
	virtual bool setConfig(const std::string &config) = 0;
	virtual bool activate() = 0;
	virtual bool teardown() {return true;}

protected:
        ControllerAbstractInterface *m_controllerInterface;
};

#endif /* GATEWAY_H */
