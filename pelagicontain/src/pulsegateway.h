/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PULSEGATEWAY_H
#define PULSEGATEWAY_H

#include <pulse/pulseaudio.h>
#include "gateway.h"

/*! Pulse audio functionality for Pelagicontain
 */
class PulseGateway :
	public Gateway
{
public:
	PulseGateway(const std::string &gatewayDir, const std::string &containerName);
	~PulseGateway();

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
	static void loadCallback(pa_context *c, uint32_t idx, void *userdata);
	static void unloadCallback(pa_context *c, int success, void *userdata);
	static void stateCallback(pa_context *c, void *userdata);

	std::string socketName();

	pa_mainloop_api *m_api;
	pa_context *m_context;
	pa_threaded_mainloop *m_mainloop;
	std::string m_socket;
	int m_index;
};

#endif /* PULSEGATEWAY_H */
