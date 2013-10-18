#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "iptables.h"
#include "pelagicontain_common.h"

int DEBUG_iptables = 1;

char *gen_iptables_rules (struct lxc_params *params)
{
	char *iptables_cmd = malloc(sizeof (char) * 1024);
	/* Execute shell script with env variable set to container IP */
	snprintf (iptables_cmd, 1024, "env SRC_IP=%s sh %s",
	          params->ip_addr, params->iptables_rule_file);

	printf ("Generating rules for IP: %s\n", params->ip_addr);
	system (iptables_cmd);
	free (iptables_cmd);
}

/*
 * This implementation is shady. What we do here is to look at the output of
 * iptables -L, look for our own IP, and then remove all rules matching our IP
 * from the FORWARD chain. There are several problems with this:
 *	- We don't lock the iptable to ensure the list we're comparing against
 *	  matches the actual list were removing from
 *	- We don't know whether we actually added the rules ourselves, or if
 *	  someone else did.
 * in short.. this function should be re-implemented in some other way where we
 * have atomic transactions for lookup and remove, and where we're also certain
 * we are actually the originators of the rule in question.
 */
void remove_iptables_rules (struct lxc_params *params)
{
	char *iptables_command = "iptables -n -L FORWARD";
	FILE *fp               = NULL;
	int   line_no          = -1; /* banner takes two lines. Start at 1 */
	char iptables_line[2048];


	fp = popen (iptables_command, "r");
	if (fp == NULL) {
		printf ("Eror executing: %s\n", iptables_command);
		return;
	}

	while (fgets (iptables_line, sizeof (iptables_line) - 1, fp) != NULL) {
		if (strstr (iptables_line, params->ip_addr) != NULL) {
			char *ipt_cmd = malloc (sizeof (char) * 100);
			if (DEBUG_iptables)
				printf ("%d > ", line_no);

			/* Actual deletion */
			snprintf(ipt_cmd, 100, 
			         "iptables -D FORWARD %d", line_no);
			system (ipt_cmd);
			free (ipt_cmd);

			line_no--; /* Removing this rule offsets the rest */
		}

		/* Print entire table */
		if (DEBUG_iptables)
			printf ("%s", iptables_line);

		line_no++;
	}

	pclose (fp);
}
