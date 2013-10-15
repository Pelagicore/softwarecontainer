#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "time.h"
#include <sys/types.h>
#include <signal.h>

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

/* Spawn the proxy and use the supplied path for the socket */
pid_t spawn_proxy (char *path, char *proxy_conf)
{
	printf ("Spawning proxy: %s\n", proxy_conf);
	pid_t pid = fork();
	if (pid == 0) { /* child */
		int exit_status = 0;
		exit_status = execlp ("dbus-proxy", "dbus-proxy", path,
		                      proxy_conf, NULL);
		printf ("Exit status of dbus proxy: %d\n", exit_status);
		exit (0);
	}
	else {
		/* parent */
	}
}

int main (int argc, int **argv)
{
	char *container_name = NULL;
	char *user_command   = NULL;
	char *lxc_command    = NULL;
	char *deploy_dir     = NULL;
	int   max_cmd_len    = sysconf(_SC_ARG_MAX);
	char  proxy_socket[1024];
	char  proxy_config[1024];
	pid_t proxy_pid      = 0;

	if (argc < 3) {
		printf ("USAGE: %s [deploy directory] [command]\n", argv[0]);
		exit(1);
	}

	/* Initialize */
	container_name = gen_ct_name();
	lxc_command    = malloc (sizeof (char) * max_cmd_len);
	deploy_dir     = (char *)argv[1];

	snprintf (proxy_socket, 1024, "%s/%s.sock", deploy_dir, container_name);
	snprintf (proxy_config, 1024, "%s/proxy_config", deploy_dir);

	/* Spawn proxy */
	proxy_pid = spawn_proxy (proxy_socket, proxy_config);

	/* Create container */
	sprintf (lxc_command, "DEPLOY_DIR=%s lxc-create -n %s -t pelagicontain"
			      " -f /etc/pelagicontain > /tmp/lxc_%s.log",
		              deploy_dir, container_name, container_name);
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

	sprintf (lxc_command, "lxc-execute -n %s -- %s",
	                      container_name, user_command);
	system (lxc_command);

	/* Destroy container */
	sprintf (lxc_command, "lxc-destroy -n %s", container_name);
	system (lxc_command);

	/* Terminate the proxy */
	printf ("Killing proxy with pid %d\n", proxy_pid);
	kill (proxy_pid, SIGTERM);

	/* .. and we're done! */
	free (user_command);
	free (lxc_command);

}
