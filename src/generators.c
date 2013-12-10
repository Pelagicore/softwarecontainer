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

#include <fstream>
#include <unistd.h>
#include <sys/time.h>
#include "generators.h"

using namespace std;

const static char *iface_counter_file = "/tmp/pelc_ifc";

char *gen_net_iface_name (const char *ip_addr_net)
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

/* Read the counter value from iface_counter_file and increase it to
 * find the next ip. Naively assumes no ip collisions occur at the moment.
 */
char *gen_ip_addr (const char *ip_addr_net)
{
	int   fd = open (iface_counter_file, O_CREAT | O_RDWR);
	int   counter = 0;
	char  buf[4];
	char *ip;

	if (fd == -1) {
		printf ("Unable to lock interface counter\n");
		return NULL;
	}
	flock (fd, LOCK_EX);

	buf[3] = 0;
	/* We reserve the first IP for gateway .. */
	if (read (fd, buf, 3) == 0) {
		counter = 2;
	} else {
		counter = atoi(buf) + 1;
		if (counter < 2 || counter > 254)
			counter = 2;
	}
	
	snprintf(buf, 4, "%03d", counter);

	/* Overwrite the first three bytes */
	lseek (fd, 0, SEEK_SET);
	write (fd, buf, 3);

	flock(fd, LOCK_UN);
	close (fd);

	ip = (char*)malloc (sizeof (char) * 20);
	snprintf(ip, 20, "%s%d", ip_addr_net, counter);

	return ip;
}

int gen_lxc_config (struct lxc_params *params)
{
	debug ("Generating config to %s for IP %s\n",
	       params->lxc_cfg_file,
	       params->ip_addr);

	/* Copy system config to temporary location */
	ifstream source(params->lxc_system_cfg, ios::binary);
	ofstream dest(params->lxc_cfg_file, ios::binary);
	dest << source.rdbuf();
	source.close();

	/* Add ipv4 parameters to config */
	dest << "lxc.network.veth.pair = " << params->net_iface_name << endl;
	dest << "lxc.network.ipv4 = " << params->ip_addr << "/24" << endl;
	dest << "lxc.network.ipv4.gateway = " << params->gw_addr << endl;

	dest.close();

	return 0;
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
