#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "time.h"
#include <sys/types.h>
#include <signal.h>
#include "pulse.h"
#include "pelagicontain_common.h"

int DEBUG_main = 1;
char *ip_addr_net = "192.168.100.";

static char  *gen_net_iface_name ();
static char  *gen_gw_ip_addr     ();
static char  *gen_ip_addr        ();
static char  *gen_lxc_config     (struct lxc_params *);
static void   initialize_config  (struct lxc_params *,  char *);
static char  *gen_ct_name        ();
static pid_t  spawn_proxy        (char *, char *, char *);

static char *gen_net_iface_name ()
{
	char *iface = malloc (sizeof (char) * 16);
	snprintf (iface, 20, "veth-%d", ip_addr_net, (rand() % 253) + 1);
	return iface;

}

static char *gen_gw_ip_addr ()
{
	char *ip = malloc (sizeof (char) * 20);;
	snprintf (ip, 20, "%s1", ip_addr_net);
	return ip;
}

static char *gen_ip_addr ()
{
	/* This is purely a proof of concept. Actually using this means we will
	 * have conflicting IPs in the containers. This will mean out IPtables
	 * rules are wrong */
	char *ip = malloc (sizeof (char) * 20);
	snprintf (ip, 20, "%s%d", ip_addr_net, (rand() % 253) + 1);
	return ip;
}

static char *gen_lxc_config (struct lxc_params *params)
{
	char cmd[1024];
	FILE *cfg;
	char *iface_line = malloc (sizeof (char) * 150);
	size_t status;

	if (DEBUG_main)
		printf ("Generating config to %s for IP %s\n",
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

static void initialize_config (struct lxc_params *ct_pars,  char *ct_base_dir)
{
	/* Initialize */
	ct_pars->lxc_system_cfg = "/etc/pelagicontain";
	ct_pars->container_name = gen_ct_name();
	ct_pars->ct_dir         = ct_base_dir;
	ct_pars->ip_addr        = gen_ip_addr ();
	ct_pars->gw_addr        = gen_gw_ip_addr ();
	ct_pars->net_iface_name = gen_net_iface_name ();
	ct_pars->tc_rate        = 500; /*kbps*/
	ct_pars->tc_peak_rate   = 500; /*kbps*/

	snprintf (ct_pars->ct_conf_dir, 1024, "%s/config/", ct_pars->ct_dir);
	snprintf (ct_pars->ct_root_dir, 1024, "%s/rootfs/", ct_pars->ct_dir);

	snprintf (ct_pars->session_proxy_socket,
		  1024,
		  "%s/sess_%s.sock",
	          ct_pars->ct_conf_dir,
	          ct_pars->container_name);

	snprintf (ct_pars->system_proxy_socket,
		  1024,
		  "%s/sys_%s.sock",
	          ct_pars->ct_conf_dir,
	          ct_pars->container_name);

	snprintf (ct_pars->deployed_session_proxy_socket,
		  1024,
	          "/deployed_app/sess_%s.sock",
		  ct_pars->container_name);

	snprintf (ct_pars->deployed_system_proxy_socket,
		  1024,
	          "/deployed_app/sys_%s.sock",
		  ct_pars->container_name);

	snprintf (ct_pars->session_proxy_config,
		  1024,
		  "%s/pelagicontain.conf",
	          ct_pars->ct_conf_dir);

	snprintf (ct_pars->system_proxy_config,
		  1024,
		  "%s/pelagicontain.conf",
	          ct_pars->ct_conf_dir);

	snprintf (ct_pars->pulse_socket,
		  1024,
		  "%s/pulse-%s.sock",
	          ct_pars->ct_conf_dir,
	          ct_pars->container_name);

	snprintf (ct_pars->deployed_pulse_socket,
		  1024,
	          "/deployed_app/pulse-%s.sock",
		  ct_pars->container_name);

	snprintf (ct_pars->lxc_cfg_file,
		  1024,
		  "/tmp/lxc_config_%s",
	          ct_pars->container_name);

	snprintf (ct_pars->iptables_rule_file,
		  1024,
		  "%s/pelagicontain.conf",
	          ct_pars->ct_conf_dir);

	snprintf (ct_pars->main_cfg_file,
		  1024,
		  "%s/pelagicontain.conf",
	          ct_pars->ct_conf_dir);

	config_initialize (ct_pars->main_cfg_file);
}

static char *gen_ct_name ()
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

/* Spawn the proxy and use the supplied path for the socket */
static pid_t spawn_proxy (char *path, char *proxy_conf, char *bus_type)
{
	if (DEBUG_main)
		printf ("Spawning proxy.. Socket: %s config: %s\n",
		         path, proxy_conf);
	pid_t pid = fork();
	if (pid == 0) { /* child */
		int exit_status = 0;
		exit_status = execlp ("dbus-proxy", "dbus-proxy", path,
		                      bus_type, proxy_conf, NULL);
		printf ("Exit status of dbus proxy: %d\n", exit_status);
		exit (0);
	}
	else {
		/* parent */
	}
}

int main (int argc, char **argv)
{
	struct lxc_params ct_pars;

	char *user_command      = NULL;
	char *lxc_command       = NULL;
	int   max_cmd_len       = sysconf(_SC_ARG_MAX);
	pid_t session_proxy_pid = 0;
	pid_t system_proxy_pid  = 0;
	char  env[4096];

	/* pulseaudio vars */
	pulse_con_t pulse;

	if (argc < 3 || argv[1][0] != '/') {
		printf ("USAGE: %s [deploy directory (abs path)] [command]\n", argv[0]);
		exit(1);
	}

	initialize_config (&ct_pars, argv[1]);
	lxc_command = malloc (sizeof (char) * max_cmd_len);

	gen_iptables_rules (&ct_pars);
	gen_lxc_config (&ct_pars);

	/* Load pulseaudio module */
	pulse_startup(&pulse, ct_pars.pulse_socket);

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
	                                 ct_pars.session_proxy_config,
	                                 "session");
	system_proxy_pid = spawn_proxy (ct_pars.system_proxy_socket,
	                                ct_pars.system_proxy_config,
	                                "system");

	/* Create container */
	sprintf (lxc_command, "DEPLOY_DIR=%s lxc-create -n %s -t pelagicontain"
		              " -f %s > /tmp/lxc_%s.log",
		              ct_pars.ct_root_dir,
		              ct_pars.container_name,
		              ct_pars.lxc_cfg_file,
		              ct_pars.container_name);
	system (lxc_command);

	/* Execute command in container */
	/* max parameter size of system */
	user_command = malloc (sizeof (char) * max_cmd_len);
	int i = 0;
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

	sprintf (lxc_command, "lxc-execute -n %s -- env %s %s",
	                      ct_pars.container_name, env, user_command);
	system (lxc_command);

	/* Destroy container */
	sprintf (lxc_command, "lxc-destroy -n %s", ct_pars.container_name);
	system (lxc_command);

	/* Terminate the proxy processes, remove sockets */
	if (DEBUG_main)
		printf ("Killing proxies with pids %d, %d\n",
		        session_proxy_pid, system_proxy_pid);
	kill (session_proxy_pid, SIGTERM);
	kill (system_proxy_pid, SIGTERM);
	remove (ct_pars.session_proxy_socket);
	remove (ct_pars.system_proxy_socket);
	remove (ct_pars.lxc_cfg_file);

	/* Remove IPTables rules */
	remove_iptables_rules (&ct_pars);

	/* Unload pulseaudio module */
	pulse_teardown(&pulse);

	/* .. and we're done! */
	free (user_command);
	free (lxc_command);
}
