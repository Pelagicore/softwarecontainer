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

#include <stdio.h>
#include "pulse.h"
#include "debug.h"

void Pulse::loadCb(pa_context *c, uint32_t idx, void *userdata)
{
	Pulse *p = static_cast<Pulse*>(userdata);
	p->module_idx = (int)idx;
	printf("pulse: Loaded module %d\n", p->module_idx);

	pa_threaded_mainloop_signal(p->mainloop, 0);
}

void Pulse::unloadCb(pa_context *c, int success, void *userdata)
{
	Pulse *p = static_cast<Pulse*>(userdata);
	if (success)
		debug ("pulse: Unloaded module %d\n", p->module_idx);
	else
		debug ("pulse: Failed to unload module %d\n", p->module_idx);

	pa_threaded_mainloop_signal(p->mainloop, 0);
}

void Pulse::stateCb(pa_context *context, void *userdata)
{
	Pulse *p = static_cast<Pulse*>(userdata);
	char socket[1031]; /* 1024 + 7 for "socket=" */

	switch (pa_context_get_state(context)) {
	case PA_CONTEXT_READY:
		debug ("Connection is up, loading module\n");
		snprintf (socket, 1031, "socket=%s", p->socket);
		pa_context_load_module (
			context,
			"module-native-protocol-unix",
			socket,
			loadCb,
			userdata);
		break;
	case PA_CONTEXT_CONNECTING:
		debug ("pulse: Connecting\n");
		break;
	case PA_CONTEXT_AUTHORIZING:
		debug ("pulse: Authorizing\n");
		break;
	case PA_CONTEXT_SETTING_NAME:
		debug ("pulse: Setting name\n");
		break;
	case PA_CONTEXT_UNCONNECTED:
		debug ("pulse: Unconnected\n");
		break;
	case PA_CONTEXT_FAILED:
		debug ("pulse: Failed\n");
		break;
	case PA_CONTEXT_TERMINATED:
		debug ("pulse: Terminated\n");
		break;
	}
}

Pulse::Pulse(char *socket) : api(0), context(0), socket(socket), module_idx(-1)
{
	/* Create mainloop */
	mainloop = pa_threaded_mainloop_new();
	pa_threaded_mainloop_start(mainloop);

	if (mainloop) {
		/* Set up connection to pulse server */
		pa_threaded_mainloop_lock(mainloop);
		api = pa_threaded_mainloop_get_api(mainloop);
		context = pa_context_new(api, "pulsetest");
		pa_context_set_state_callback (context, stateCb, this);

		int err = pa_context_connect (
			context,        /* context */
			NULL,              /* default server */ 
			PA_CONTEXT_NOFAIL, /* keep reconnection on failure */
			NULL );            /* use default spawn api */

		if (err != 0) {
			fprintf (stderr, "Pulse error: %s\n", pa_strerror(err));
		}
		pa_threaded_mainloop_unlock (mainloop);
	} else {
		fprintf (stderr, "Failed to create pulse mainloop->\n");
	}
}

Pulse::~Pulse()
{
	if (mainloop) {
		/* Unload module if module loaded successfully on startup */
		if (module_idx != -1)
		{
			pa_threaded_mainloop_lock(mainloop);
			pa_context_unload_module(
				context,
				module_idx,
				unloadCb,
				this);
			pa_threaded_mainloop_wait(mainloop);
			pa_threaded_mainloop_unlock(mainloop);
		}

		/* Release pulse */
		pa_threaded_mainloop_lock(mainloop);
		pa_context_disconnect(context);
		pa_context_unref(context);
		pa_threaded_mainloop_unlock(mainloop);

		pa_threaded_mainloop_stop(mainloop);
		pa_threaded_mainloop_free(mainloop);
	}
	debug ("pulse: Teardown complete\n");
}
