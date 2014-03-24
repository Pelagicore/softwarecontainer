/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DEVICENODEGATEWAY_H
#define DEVICENODEGATEWAY_H

#include "gateway.h"
#include "controllerinterface.h"

class DeviceNodeGateway : public Gateway
{
public:
	DeviceNodeGateway(ControllerAbstractInterface *controllerInterface);
	~DeviceNodeGateway(){}

	/*!
	 *  Implements Gateway::id
	 */
	virtual std::string id();

	/*!
	 *  Implements Gateway::setConfig
	 */
	virtual bool setConfig(const std::string &config);

	/*!
	 *  Implements Gateway::activate
	 */
	virtual bool activate();

	/*! Implements Gateway::environment
	 */
	virtual std::string environment();

private:
};

#endif /* DEVICENODEGATEWAY_H */
