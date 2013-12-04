/*
 * Copyright (C) 2013, Pelagicore AB <jonatan.palsson@pelagicore.com>
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
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "pulse.h"
#include "pelagicontain_common.h"
#include "config.h"
#include "debug.h"
#include "generators.h"
#include "iptables.h"
#include "trafficcontrol.h"

/*! \brief Initialize config struct
 *
 * Initialize the config struct with various paths and parameters. Some of
 * these parameters are read som $container/config/pelagicore.conf. This
 * function verifies that all values parsed from this config file are present,
 * and returns -EINVAL if they are not.
 *
 * \param  *ct_pars     Pointer to struct to initialize
 * \param  *ct_base_dir Path to container base dir
 * \return 0            Upon success
 * \return -EINVAL      Upon bad/missing configuration parameters
 */
static int initialize_config  (struct lxc_params *,  char *);

/*! \brief Spawn the proxy and use the supplied path for the socket
 *
 * \param  path       path to the socket file to use. File is created.
 * \param  proxy_conf path to configuration file for proxy
 * \param  bus_type   "session" or "system"
 * \return -1         Upon failure of forking
 * \return PID of spawned process on success
 */
static pid_t spawn_proxy (char *, char *, const char *);


static int initialize_config (struct lxc_params *ct_pars,  char *ct_base_dir)
{
	char *ip_addr_net = NULL;

	/* Initialize */
	ct_pars->container_name = gen_ct_name();
	ct_pars->ct_dir         = ct_base_dir;

	snprintf (ct_pars->ct_conf_dir, 1024, "%s/config/", ct_pars->ct_dir);
	snprintf (ct_pars->ct_root_dir, 1024, "%s/rootfs/", ct_pars->ct_dir);

	snprintf (ct_pars->session_proxy_socket,
		  1024,
		  "%s/sess_%s.sock",
	          ct_pars->ct_root_dir,
	          ct_pars->container_name);

	snprintf (ct_pars->system_proxy_socket,
		  1024,
		  "%s/sys_%s.sock",
	          ct_pars->ct_root_dir,
	          ct_pars->container_name);

	snprintf (ct_pars->deployed_session_proxy_socket,
		  1024,
	          "/deployed_app/sess_%s.sock",
		  ct_pars->container_name);

	snprintf (ct_pars->deployed_system_proxy_socket,
		  1024,
	          "/deployed_app/sys_%s.sock",
		  ct_pars->container_name);

	snprintf (ct_pars->pulse_socket,
		  1024,
		  "%s/pulse-%s.sock",
	          ct_pars->ct_root_dir,
	          ct_pars->container_name);

	snprintf (ct_pars->deployed_pulse_socket,
		  1024,
	          "/deployed_app/pulse-%s.sock",
		  ct_pars->container_name);

	snprintf (ct_pars->lxc_cfg_file,
		  1024,
		  "/tmp/lxc_config_%s",
	          ct_pars->container_name);

	snprintf (ct_pars->main_cfg_file,
		  1024,
		  "%s/pelagicontain.conf",
	          ct_pars->ct_conf_dir);

	config_initialize (ct_pars->main_cfg_file);

	ct_pars->lxc_system_cfg = config_get_string ("lxc-config-template");
	if (!ct_pars->lxc_system_cfg) {
		printf ("Unable to read lxc-config-template from config!\n");
		return -EINVAL;
	}

	ct_pars->tc_rate = config_get_string ("bandwidth-limit");
	if (!ct_pars->tc_rate) {
		printf ("Unable to read bandwidth-limit from config!\n");
		return -EINVAL;
	}

	ip_addr_net = config_get_string ("ip-addr-net");
	if (!ip_addr_net) {
		printf ("Unable to read ip-addr-net from config!\n");
		return -EINVAL;
	}

	ct_pars->gw_addr = config_get_string ("gw-ip-addr");
	if (!ct_pars->gw_addr) {
		printf ("Unable to read gw-ip-addr from config!\n");
		return -EINVAL;
	}

	ct_pars->ip_addr        = gen_ip_addr (ip_addr_net);
	ct_pars->net_iface_name = gen_net_iface_name (ip_addr_net);

	return 0;
}

static pid_t spawn_proxy (char *path, char *proxy_conf, const char *bus_type)
{
	pid_t pid;

	debug ("Spawning proxy.. Socket: %s config: %s\n",
	       path, proxy_conf);

	pid = fork();

	if (pid == 0) { /* child */
		int exit_status = 0;
		exit_status = execlp ("dbus-proxy", "dbus-proxy", path,
		                      bus_type, proxy_conf, NULL);
		printf ("Exit status of dbus proxy: %d\n", exit_status);
		exit (0);
	}
	else {
		/* parent */
		if (pid == -1) 
			printf ("Failed to spawn D-Bus proxy!\n");
		return pid;
	}
}

int main (int argc, char **argv)
{
	struct lxc_params ct_pars;

	int   max_cmd_len       = sysconf(_SC_ARG_MAX);
	char  user_command[max_cmd_len];
	char  lxc_command[max_cmd_len];
	pid_t session_proxy_pid = 0;
	pid_t system_proxy_pid  = 0;
	char  env[4096];

	if (argc < 3 || argv[1][0] != '/') {
		printf ("USAGE: %s [deploy directory (abs path)] [command]\n", argv[0]);
		return -1;
	}

	if (initialize_config (&ct_pars, argv[1])) {
		printf ("Failed to initialize config. Exiting\n");
		return -1;
	}

	gen_iptables_rules (&ct_pars);
	gen_lxc_config (&ct_pars);

	/* Load pulseaudio module */
	Pulse pulse(ct_pars.pulse_socket);

	/* Limit network interface */
	limit_iface (&ct_pars);

	/* Set up an environment */
	strcat (env, "DBUS_SESSION_BUS_ADDRESS=unix:path=");
	strcat (env, ct_pars.deployed_session_proxy_socket);
	strcat (env, " ");
	strcat (env, "DBUS_SYSTEM_BUS_ADDRESS=unix:path=");
	strcat (env, ct_pars.deployed_system_proxy_socket);
	strcat (env, " ");
	strcat (env, "PULSE_SERVER=");
	strcat (env, ct_pars.deployed_pulse_socket);

	/* Spawn proxy */
	session_proxy_pid = spawn_proxy (ct_pars.session_proxy_socket,
	                                 ct_pars.main_cfg_file,
	                                 "session");
	if (session_proxy_pid == -1) {
		printf ("Failed to spawn session D-Bus proxy. Exiting\n");
		return -1;
	}

	system_proxy_pid = spawn_proxy (ct_pars.system_proxy_socket,
	                                ct_pars.main_cfg_file,
	                                "system");
	if (system_proxy_pid == -1) {
		printf ("Failed to spawn system D-Bus proxy. Exiting\n");
		return -1;
	}

	/* Create container */
	sprintf (lxc_command, "DEPLOY_DIR=%s lxc-create -n %s -t pelagicontain"
		              " -f %s > /tmp/lxc_%s.log",
		              ct_pars.ct_root_dir,
		              ct_pars.container_name,
		              ct_pars.lxc_cfg_file,
		              ct_pars.container_name);
	system (lxc_command);

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
	          ct_pars.container_name, env, user_command);
	system (lxc_command);

	/* Destroy container */
	snprintf (lxc_command, max_cmd_len, "lxc-destroy -n %s",
	          ct_pars.container_name);
	system (lxc_command);

	/* Terminate the proxy processes, remove sockets */
	debug ("Killing proxies with pids %d, %d\n",
	       session_proxy_pid, system_proxy_pid);

	if (kill (session_proxy_pid, SIGTERM) == -1)
		printf ("Failed to kill session proxy!\n");
	if (kill (system_proxy_pid, SIGTERM) == -1)
		printf ("Failed to kill system proxy!\n");

	if (remove (ct_pars.session_proxy_socket) == -1)
		printf ("Failed to remove session proxy socket!\n");
	if (remove (ct_pars.system_proxy_socket) == -1)
		printf ("Failed to remove system proxy socket!\n");
	if (remove (ct_pars.lxc_cfg_file) == -1)
		printf ("Failed to remove lxc config file!\n");

	/* Remove IPTables rules */
	if (remove_iptables_rules (&ct_pars))
		printf ("Failed to remove IPTables rules!\n");
}
