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

#include <unistd.h>
#include "container.h"
#include "generators.h"

const char *Container::name()
{
	return m_name.c_str();
}

int Container::run(int argc, char **argv, struct lxc_params *ct_pars)
{
	int  max_cmd_len = sysconf(_SC_ARG_MAX);
	char lxc_command[max_cmd_len];
	char user_command[max_cmd_len];
	char env[4096];

	/* Set up an environment */
	strcat (env, "DBUS_SESSION_BUS_ADDRESS=unix:path=");
	strcat (env, ct_pars->deployed_session_proxy_socket);
	strcat (env, " ");
	strcat (env, "DBUS_SYSTEM_BUS_ADDRESS=unix:path=");
	strcat (env, ct_pars->deployed_system_proxy_socket);
	strcat (env, " ");
	strcat (env, "PULSE_SERVER=");
	strcat (env, ct_pars->deployed_pulse_socket);

	/* Create container */
	sprintf (lxc_command, "DEPLOY_DIR=%s lxc-create -n %s -t pelagicontain"
                              " -f %s > /tmp/lxc_%s.log",
                              ct_pars->ct_root_dir,
                              name(),
                              ct_pars->lxc_cfg_file,
                              name());
	int ret = system (lxc_command);
	if (ret)
		printf("%s returned %d\n", lxc_command, ret);

	/* Execute command in container */
	int i;
	for (i = 2; i < argc; i++) {
		int clen = strlen (user_command);
		int nlen = strlen ((const char *) argv[i]);
		if (nlen + clen >= max_cmd_len - 256) {
			printf ("Parameter list too long\n");
			exit (1);
		}
		strcat (user_command, (const char *)argv[i]);
		strcat (user_command, " ");
	}

	snprintf (lxc_command, max_cmd_len, "lxc-execute -n %s -- env %s %s",
		  name(), env, user_command);
	ret = system (lxc_command);
	if (ret)
		printf("%s returned %d\n", lxc_command, ret);

	/* Destroy container */
	snprintf (lxc_command, max_cmd_len, "lxc-destroy -n %s", name());
	ret = system (lxc_command);
        if (ret)
                printf("%s returned %d\n", lxc_command, ret);
}

Container::Container(struct lxc_params *ct_pars)
{
	m_name = ct_pars->container_name;

	/* Initialize DBus socket paths */
	snprintf(ct_pars->session_proxy_socket, 1024,
		"%s/sess_%s.sock", ct_pars->ct_root_dir, name());
	snprintf(ct_pars->system_proxy_socket, 1024,
		"%s/sys_%s.sock", ct_pars->ct_root_dir, name());
	snprintf(ct_pars->deployed_session_proxy_socket, 1024,
		"/deployed_app/sess_%s.sock", name());
	snprintf(ct_pars->deployed_system_proxy_socket, 1024,
		"/deployed_app/sys_%s.sock", name());

	/* Initialize pulse socket paths */
	snprintf(ct_pars->pulse_socket, 1024,
		"%s/pulse-%s.sock", ct_pars->ct_root_dir, name());
	snprintf(ct_pars->deployed_pulse_socket, 1024,
		"/deployed_app/pulse-%s.sock", name());

	/* Initialize LXC configuration file name */
	snprintf(ct_pars->lxc_cfg_file, 1024,
		"/tmp/lxc_config_%s", name());
}

Container::~Container()
{
}
