/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DEVICENODEGATEWAY_H
#define DEVICENODEGATEWAY_H

#include "jansson.h"

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
    struct Device {
        std::string name;
        std::string major;
        std::string minor;
        std::string mode;
    };

    std::vector<struct DeviceNodeGateway::Device> m_devList;

    std::vector<struct Device> parseDeviceList(json_t *list, bool &ok);

    ControllerAbstractInterface  *m_controllerIface;
};

#endif /* DEVICENODEGATEWAY_H */
