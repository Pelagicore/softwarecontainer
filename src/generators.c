#include "generators.h"

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
	/* This is purely a proof of concept. Actually using this means we will
	 * have conflicting IPs in the containers. This will mean out IPtables
	 * rules are wrong */
	char *ip = malloc (sizeof (char) * 20);
	snprintf (ip, 20, "%s%d", ip_addr_net, (rand() % 253) + 1);
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
