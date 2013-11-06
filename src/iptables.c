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

#include "iptables.h"

int gen_iptables_rules (struct lxc_params *params)
{
	char *iptables_cmd        = NULL;
	char *iptables_rules      = NULL;
	char *iptables_rules_file = strdup ("/tmp/iptables_rules_XXXXXX");
	int   iptf                = NULL;
	int   retval              = 0;
	      
	iptables_cmd = malloc(sizeof (char) * 1024);
	if (!iptables_cmd) {
		printf ("Unable to allocate memory for iptables_cmd\n");
		retval = -EINVAL;
		goto cleanup;
	}

	iptables_rules = config_get_string ("iptables-rules");
	if (!iptables_rules) {
		printf ("Unable to retrieve value for key 'iptables-rules'\n");
		retval = -EINVAL;
		goto cleanup;
	}

	iptf = mkstemp (iptables_rules_file);
	if (iptf == -1) {
		printf ("Unable to open %s\n", iptables_rules_file);
		retval = -EIO;
		goto cleanup;
	}

	if (write (iptf, iptables_rules,
		   sizeof (char) * strlen (iptables_rules)) == -1) {
		printf ("Failed to write rules file\n");
		retval = -EIO;
		goto cleanup;
	}

	if (close (iptf) == 1) {
		printf ("Failed to close rules file\n");
		retval = -EIO;
		goto cleanup;
	}

	/* Execute shell script with env variable set to container IP */
	snprintf (iptables_cmd, 1024, "env SRC_IP=%s sh %s",
	          params->ip_addr, iptables_rules_file);

	printf ("Generating rules for IP: %s\n", params->ip_addr);
	if (system (iptables_cmd) == -1) {
		printf ("Failed to execute iptables command\n");
		retval = -EINVAL;
		goto cleanup;
	}

cleanup:
	unlink (iptables_rules_file);
	free (iptables_cmd);
	free (iptables_rules);
	free (iptables_rules_file);

	return retval;
}

void remove_iptables_rules (struct lxc_params *params)
{
	char *iptables_command = "iptables -n -L FORWARD";
	FILE *fp               = NULL;
	int   line_no          = -1; /* banner takes two lines. Start at 1 */
	char iptables_line[2048];


	fp = popen (iptables_command, "r");
	if (fp == NULL) {
		printf ("Eror executing: %s\n", iptables_command);
		goto cleanup;
	}

	while (fgets (iptables_line, sizeof (iptables_line) - 1, fp) != NULL) {
		if (strstr (iptables_line, params->ip_addr) != NULL) {
			char *ipt_cmd = malloc (sizeof (char) * 100);
			if (!ipt_cmd) {
				printf ("Failed to allocate memory "
					"for ipt_cmd\n");
				break; /* No memory was allocated */
			}
			debug ("%d > ", line_no);

			/* Actual deletion */
			snprintf(ipt_cmd, 100, 
			         "iptables -D FORWARD %d", line_no);
			if (system (ipt_cmd) == -1)
				printf ("Failed to execute '%s'\n", ipt_cmd);
				/* We'll continue trying with the rest */

			free (ipt_cmd);

			line_no--; /* Removing this rule offsets the rest */
		}

		/* Print entire table */
		debug ("%s", iptables_line);

		line_no++;
	}

cleanup:
	if (fp)
		pclose (fp);
}
