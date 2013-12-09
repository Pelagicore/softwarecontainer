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
#include "trafficcontrol.h"

/*! \brief Poll for a device
 *
 * Poll getifaddrs for a specific interface, and give up after a few seconds
 *
 * \param iface name of the interface to look for
 * \return 1       if the device is found
 * \return 0       if the device is not found
 * \return -EINVAL if getifaddrs fails
 */
static int wait_for_device (const char *iface) 
{
	struct ifaddrs *ifaddr, *ifa;
	int   max_poll    = 10;
	const char *found_iface = NULL;
	int   i           = 0;
	int retval        = 0;

	while (i < max_poll && found_iface == NULL) {
		if (getifaddrs(&ifaddr) == -1) {
			retval = -EINVAL;
			perror("getifaddrs");
			goto cleanup_wait;
		}

		/* Iterate through the device list */
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_name == NULL) {
				continue;
			} else {
				if (strcmp (ifa->ifa_name, iface) == 0) {
					debug ("Device found: %s\n", 
						 ifa->ifa_name);
					found_iface = iface;
					break;
				}
			}
		}

		if (!iface)
			debug ("Device unavailable");

		/* Give the device some time to show up */
		usleep (250000);
		i++;
	}

	retval = (found_iface != NULL);
cleanup_wait:
	return retval;
}

/*
 * This is a wrapper around the tc command. This function will issue system ()
 * calls to tc.
 * */
int limit_iface (const char *net_iface_name, const char *tc_rate)
{
	pid_t pid = 0;

	/* fork */
	pid = fork();
	if (pid == 0) { /* child */
		char cmd[256];

		snprintf (cmd, 256, "tc qdisc "
					"add "
					"dev "
					"%s "
					"root "
					"tbf "
					"rate %s "
					"burst 5kb "
					"latency 70ms ",
					net_iface_name,
					tc_rate);

		/* poll for device */
		if (!wait_for_device (net_iface_name)) {
			printf ("Device never showed up. Not setting TC.\n");
			/* We're forked, so just exit */
			exit (0);
		}

		/* issue command */
		debug ("issuing: %s\n", cmd);

		system (cmd);

		exit (0);

	} else { /* parent */
		if (pid == -1) {
			printf ("Unable to fork interface observer!\n");
			return -EINVAL;
		}
		return 0;
	}

}


/*
 * This function issues "tc qdisc del dev <device> root"
 */
int clear_iface_limits (char *iface)
{
	int retval = 0;
	char cmd[256];

	snprintf (cmd, 256, "tc qdisc "
				"del "
				"dev "
				"%s "
				"root ",
				iface);

	if (system (cmd) == -1) {
		printf ("Unable to execute limit clear command\n");
		return -EINVAL;
	}

	return retval;
}
