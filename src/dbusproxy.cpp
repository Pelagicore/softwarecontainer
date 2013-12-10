/*
 * Copyright (C) 2013, Pelagicore AB <tomas.hallenberg@pelagicore.com>
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
#include <sys/types.h>
#include <signal.h>
#include "dbusproxy.h"
#include "debug.h"

DBusProxy::DBusProxy(const char *socket, const char *config, const char *type):
	m_socket(socket), m_type(type)
{
	debug("Spawning %s proxy, socket: %s, config: %s\n",
		type, socket, config);

	m_pid = fork();
	if (m_pid == 0) { /* child */
		/* This call never returns... */
		execlp ("dbus-proxy", "dbus-proxy", socket, type, config, NULL);
	} else {
		if (m_pid == -1)
			printf("Failed to spawn DBus proxy!\n");
	}
}

DBusProxy::~DBusProxy()
{
	if (kill (m_pid, SIGTERM) == -1) {
		printf("Failed to kill %s proxy!\n", m_type);
	} else {
		debug("Killed %s proxy!\n", m_type);
	}

	if (remove (m_socket) == -1) {
		printf("Failed to remove %s proxy socket!\n", m_type);
	} else {
		debug("Removed %s proxy socket!\n", m_type);
	}
}
