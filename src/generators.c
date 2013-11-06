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

#include "generators.h"

const static char *iface_counter_file = "/tmp/pelc_ifc";

char *gen_net_iface_name (char *ip_addr_net)
{
	char *iface = malloc (sizeof (char) * 16);
	snprintf (iface, 20, "veth-%d", ip_addr_net, (rand() % 253) + 1);
	return iface;
}

char *gen_gw_ip_addr (char *ip_addr_net)
{
	char *ip = malloc (sizeof (char) * 20);;
	snprintf (ip, 20, "%s1", ip_addr_net);
	return ip;
}

char *gen_ip_addr (char *ip_addr_net)
{
	char *ip    = malloc (sizeof (char) * 20);
	char *buf   = calloc(sizeof (char), 4);
	int fd      = open (iface_counter_file, O_CREAT | O_RDWR);
	int counter = 0;

	if (fd == -1) {
		printf ("Unable to lock interface counter\n");
		return NULL;
	}
	flock (fd, LOCK_EX);

	if (read (fd, buf, 3) == 0)
		counter = 1;
	else
		counter = atoi (buf);

	snprintf(buf, 4, "%03d", (counter % 254) + 1);

	lseek (fd, 0, SEEK_SET);
	write (fd, buf, 3);

	flock(fd, LOCK_UN);
	close (fd);

	free (buf);
	snprintf (ip, 20, "%s%d", ip_addr_net, counter);
	return ip;
}

char *gen_lxc_config (struct lxc_params *params)
{
	char cmd[1024];
	FILE *cfg;
	char *iface_line = malloc (sizeof (char) * 150);
	size_t status;

	debug ("Generating config to %s for IP %s\n",
	       params->lxc_cfg_file,
	       params->ip_addr);

	/* copy system config to temporary location */
	snprintf (cmd, 1024, "cp %s %s", params->lxc_system_cfg,
	                                 params->lxc_cfg_file);
	system (cmd);

	/* Add ipv4 config to config */
	snprintf (iface_line, 150, "lxc.network.veth.pair = %s\n"
	                           "lxc.network.ipv4 = %s/24\n"
	                           "lxc.network.ipv4.gateway = %s\n",
	                           params->net_iface_name,
	                           params->ip_addr,
	                           params->gw_addr);

	cfg = fopen (params->lxc_cfg_file, "a+");
	status = fwrite (iface_line,
	                 sizeof (char) * strlen (iface_line),
	                 1,
	                 cfg);
	fclose (cfg);

	free (iface_line);
}

char *gen_ct_name ()
{
	struct timeval time;
	gettimeofday(&time,NULL);
	char *name = malloc (sizeof (char) * 10);
	int   i    = 0;
	srand((time.tv_sec * 1000) + (time.tv_usec / 1000));
	static const char alphanum[] =
        "abcdefghijklmnopqrstuvwxyz";

	for (i = 0; i < 10; i++) {
		name[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	name[9] = 0;
	return name;
}
