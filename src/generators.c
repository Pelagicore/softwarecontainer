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

#include <unistd.h>
#include <sys/time.h>
#include "generators.h"

const static char *iface_counter_file = "/tmp/pelc_ifc";

char *gen_net_iface_name (char *ip_addr_net)
{
	struct  ifaddrs *ifaddr, *ifa;
	char   *iface     = NULL;
	int     collision = 0;

	iface = (char*)malloc (sizeof (char) * 16);
	if (!iface) {
		printf ("Failed to malloc iface\n");
		return NULL;
	}

	do {
		snprintf (iface, 20, "veth-%d", (rand() % 1024));

		if (getifaddrs(&ifaddr) == -1) {
			perror("getifaddrs");
			exit(EXIT_FAILURE);
		}

		/* Iterate through the device list */
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_name == NULL) {
				continue;
			} else {
				if (strcmp (ifa->ifa_name, iface) == 0) {
					collision = 1;
					break;
				}
			}
		}
	} while (collision);

	return iface;
}

char *gen_ip_addr (char *ip_addr_net)
{
	char *ip = (char*)malloc (sizeof (char) * 20);
	char  buf[4];
	int   fd      = open (iface_counter_file, O_CREAT | O_RDWR);
	int   counter = 0;

	if (fd == -1) {
		printf ("Unable to lock interface counter\n");
		return NULL;
	}
	flock (fd, LOCK_EX);

	if (read (fd, buf, 3) == 0)
		counter = 1;
	else
		counter = atoi (buf);

	/* We reserve the first IP for gateway .. */
	snprintf(buf, 4, "%03d", (counter % 254) + 2);

	/* Overwrite the first three bytes */
	lseek (fd, 0, SEEK_SET);
	write (fd, buf, 3);

	flock(fd, LOCK_UN);
	close (fd);

	snprintf (ip, 20, "%s%d", ip_addr_net, counter);
	return ip;
}

int gen_lxc_config (struct lxc_params *params)
{
	FILE  *cfg;
	char   cmd[1024];
	size_t status;
	int    retval     = 0;
	char   iface_line[150];

	debug ("Generating config to %s for IP %s\n",
	       params->lxc_cfg_file,
	       params->ip_addr);

	/* copy system config to temporary location */
	snprintf (cmd, 1024, "cp %s %s", params->lxc_system_cfg,
	                                 params->lxc_cfg_file);
	if (system (cmd) == -1) {
		printf ("Failed to copy config to temp location\n");
		retval = -EINVAL;
		goto cleanup_config;
	}

	/* Add ipv4 config to config */
	snprintf (iface_line, 150, "lxc.network.veth.pair = %s\n"
	                           "lxc.network.ipv4 = %s/24\n"
	                           "lxc.network.ipv4.gateway = %s\n",
	                           params->net_iface_name,
	                           params->ip_addr,
	                           params->gw_addr);

	cfg    = fopen (params->lxc_cfg_file, "a+");
	if (!cfg) {
		printf ("Failed to open temp config file!\n");
		retval = -EINVAL;
		goto cleanup_config;
	}

	status = fwrite (iface_line,
	                 sizeof (char) * strlen (iface_line),
	                 1,
	                 cfg);
cleanup_config:
	fclose (cfg);
	return retval;
}

char *gen_ct_name ()
{
	struct  timeval time;
	char   *name;
	int     i                    = 0;
	static const char alphanum[] = "abcdefghijklmnopqrstuvwxyz";

	name = (char*)malloc (sizeof (char) * 10);
	if (!name) {
		printf ("Failed to malloc name in gen_ct_name\n");
		return NULL;
	}

	gettimeofday (&time, NULL);
	srand ((time.tv_sec * 1000) + (time.tv_usec / 1000));

	for (i = 0; i < 10; i++) {
		name[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	name[9] = '\0';
	return name;
}
