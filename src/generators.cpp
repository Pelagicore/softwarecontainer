/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "ifaddrs.h"

#include "generators.h"
#include "debug.h"

using namespace std;

const static char *iface_counter_file = "/tmp/pelc_ifc";

std::string gen_net_iface_name (const char *ip_addr_net)
{
	struct  ifaddrs *ifaddr, *ifa;
	int collision = 0;
	char iface[16];

	do {
		snprintf (iface, sizeof(iface), "veth-%d", (rand() % 1024));

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

	return std::string(iface);
}

/*
 * Read the counter value from iface_counter_file and increase it to
 * find the next ip. Naively assumes no ip collisions occur at the moment.
 */
std::string gen_ip_addr (const char *ip_addr_net)
{
	int  fd = open (iface_counter_file, O_CREAT | O_RDWR);
	int  counter = 0;
	char buf[4];
	char ip[20];

	if (fd == -1) {
		log_error ("Unable to lock interface counter");
		return std::string();
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
	
	snprintf(buf, sizeof(buf), "%03d", counter);

	/* Overwrite the first three bytes */
	lseek (fd, 0, SEEK_SET);
	write (fd, buf, 3);

	flock(fd, LOCK_UN);
	close (fd);

	snprintf(ip, sizeof(ip), "%s%d", ip_addr_net, counter);

	return std::string(ip);
}

std::string gen_ct_name()
{
	static const char alphanum[] = "abcdefghijklmnopqrstuvwxyz";
	struct timeval time;
	char name[10];

	gettimeofday (&time, NULL);
	srand ((time.tv_sec * 1000) + (time.tv_usec / 1000));

	for (int i = 0; i < 9; i++) {
		name[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	name[9] = '\0';
	return std::string(name);
}
