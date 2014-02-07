/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "debug.h"
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
			log_debug("%d > ", line_no);

			/* Actual deletion */
			snprintf(ipt_cmd, sizeof(ipt_cmd), "iptables -D FORWARD %d", line_no);
			if (system(ipt_cmd) == -1)
				log_error ("Failed to execute '%s'", ipt_cmd);
				/* We'll continue trying with the rest */

			line_no--; /* Removing this rule offsets the rest */
		}

		/* Print entire table */
		log_debug("%s", iptables_line);

		line_no++;
	}
	pclose(fp);
}
