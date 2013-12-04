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
 *  \author Erik Botö (erik.boto@pelagicore.com)
 *  \file pulse.h
 *
 *  Pulse audio functionality for Pelagicontain
 */

#ifndef _pulse_h_
#include <pulse/pulseaudio.h>

typedef struct pulse_con {
	pa_mainloop_api * api;
	pa_context * context;
	pa_threaded_mainloop * mainloop;
	char * socket;
	int module_idx;
} pulse_con_t;

void pulse_startup(pulse_con_t *p, char * socket);
void pulse_teardown (pulse_con_t *p);

#endif /* _pulse_h_*/
