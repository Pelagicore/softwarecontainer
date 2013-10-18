#ifndef PELAGICONTAIN_COMMON_H
#define PELAGICONTAIN_COMMON_H

struct lxc_params {
	char *ip_addr;
	char *container_name;
	char *lxc_system_cfg;
	char iptables_rule_file[1024];
	char lxc_cfg_file[1024];
};

#endif /* PELAGICONTAIN_COMMON_H */
