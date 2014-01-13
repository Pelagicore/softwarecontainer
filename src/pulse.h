/*
 * Copyright (C) 2013, Pelagicore AB <erik.boto@pelagicore.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

/*! \brief Pulse functionality
 *  \file pulse.h
 *
 *  Pulse audio functionality for Pelagicontain
 */

#ifndef PULSE_H
#define PULSE_H

#include <pulse/pulseaudio.h>
#include "gateway.h"

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
