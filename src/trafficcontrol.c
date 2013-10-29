#include "trafficcontrol.h"
#include "pelagicontain_common.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "ifaddrs.h"
#include "string.h"

int DEBUG_tc = 1;

int _wait_for_device (char *iface) 
{
	struct ifaddrs *ifaddr, *ifa;
	int   max_poll    = 10;
	char *found_iface = NULL;
	int   i           = 0;
	while (i < max_poll && found_iface == NULL) {
		if (getifaddrs(&ifaddr) == -1) {
			perror("getifaddrs");
			exit(EXIT_FAILURE);
		}

		/* Iterate through the device list */
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_name == NULL)
				continue;
			else {
				if (strcmp (ifa->ifa_name, iface) == 0) {
					if (DEBUG_tc)
						printf ("Device found: %s\n", 
						 ifa->ifa_name);
					found_iface = iface;
					break;
				}
			}
		}

		if (!iface && DEBUG_tc)
			printf ("Device unavailable");

		/* Give the device some time to show up */
		usleep (250000);
		i++;
	}

	return (found_iface != NULL);
}

/*
 * This is a wrapper around the tc command. This function will issue system ()
 * calls to tc.
 * */
void limit_iface (struct lxc_params *params)
{
	pid_t pid = 0;

	/* fork */
	pid = fork();
	if (pid == 0) { /* child */
		char *cmd      = malloc (sizeof (char) * 256);

		snprintf (cmd, 256, "tc qdisc "
					"add "
					"dev "
					"%s "
					"root "
					"tbf "
					"rate %s "
					"burst 5kb "
					"latency 70ms ",
					params->net_iface_name,
					params->tc_rate);

		/* poll for device */
		if (!_wait_for_device (params->net_iface_name)) {
			printf ("Device never showed up. Not setting TC.\n");
			exit (0);
		}

		/* issue command */
		if (DEBUG_tc)
			printf ("issuing: %s\n", cmd);

		system (cmd);
		free (cmd);
		exit (0);

	} else { /* parent */
		return;
	}

}


/*
 * This function issues "tc qdisc del dev <device> root"
 */
void clear_iface_limits (char *iface)
{
		char *cmd      = malloc (sizeof (char) * 256);

		snprintf (cmd, 256, "tc qdisc "
					"del "
					"dev "
					"%s "
					"root ",
					iface);

		system (cmd);
		free (cmd);

}
