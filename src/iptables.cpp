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

IpTables::IpTables(const char *ip_addr, const char *iptables_rules) :
	m_ip(ip_addr)
{
	char iptables_cmd[1024];
	char iptables_rules_file[] = "/tmp/iptables_rules_XXXXXX";
	int iptf = 0;

	iptf = mkstemp(iptables_rules_file);
	if (iptf == -1) {
		log_error("Unable to open %s", iptables_rules_file);
		return;
	}

	if (write(iptf, iptables_rules, sizeof(char) * strlen(iptables_rules)) == -1) {
		log_error("Failed to write rules file");
		goto unlink_file;
	}

	if (close(iptf) == 1) {
		log_error("Failed to close rules file");
		goto unlink_file;
	}

	/* Execute shell script with env variable set to container IP */
	snprintf(iptables_cmd, sizeof(iptables_cmd), "env SRC_IP=%s sh %s",
		m_ip, iptables_rules_file);

	log_error("Generating rules for IP: %s", m_ip);
	if (system(iptables_cmd) == -1)
		log_error("Failed to execute iptables command");

unlink_file:
	unlink(iptables_rules_file);
}

IpTables::~IpTables()
{
	const char *iptables_command = "iptables -n -L FORWARD";
	FILE *fp = NULL;
	int line_no = -1; /* banner takes two lines. Start at 1 */
	char iptables_line[2048];

	fp = popen(iptables_command, "r");
	if (fp == NULL) {
		log_error("Error executing: %s", iptables_command);
		return;
	}

	while (fgets(iptables_line, sizeof(iptables_line) - 1, fp) != NULL) {
		if (strstr(iptables_line, m_ip) != NULL) {
			char ipt_cmd[100];
			debug("%d > ", line_no);

			/* Actual deletion */
			snprintf(ipt_cmd, sizeof(ipt_cmd), "iptables -D FORWARD %d", line_no);
			if (system(ipt_cmd) == -1)
				log_error ("Failed to execute '%s'", ipt_cmd);
				/* We'll continue trying with the rest */

			line_no--; /* Removing this rule offsets the rest */
		}

		/* Print entire table */
		debug("%s", iptables_line);

		line_no++;
	}
	pclose(fp);
}
