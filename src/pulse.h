/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PULSE_H
#define PULSE_H

#include <pulse/pulseaudio.h>
#include "gateway.h"

/*! \brief Pulse functionality
 *  \file pulse.h
 *
 *  Pulse audio functionality for Pelagicontain
 */
class Pulse :
	public Gateway
{
public:
	Pulse(const char *socket);
	~Pulse();

	std::string environment();

private:
	static void loadCallback(pa_context *c, uint32_t idx, void *userdata);
	static void unloadCallback(pa_context *c, int success, void *userdata);
	static void stateCallback(pa_context *c, void *userdata);

	const char *socketName();

	pa_mainloop_api *m_api;
	pa_context *m_context;
	pa_threaded_mainloop *m_mainloop;
	const char *m_socket;
	int m_index;
};

#endif /* PULSE_H */
