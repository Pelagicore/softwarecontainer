#ifndef PELAGICONTAIN_COMMON_H
#define PELAGICONTAIN_COMMON_H

struct lxc_params {
	char *ip_addr;
	char *gw_addr;
	char *container_name;
	char *lxc_system_cfg;
	char *deploy_dir;
	char iptables_rule_file[1024];
	char lxc_cfg_file[1024];
	char session_proxy_socket[1024];
	char system_proxy_socket[1024];
	char deployed_session_proxy_socket[1024];
	char deployed_system_proxy_socket[1024];
	char pulse_socket[1024];
	char deployed_pulse_socket[1024];
	char session_proxy_config[1024];
	char system_proxy_config[1024];
};

#endif /* PELAGICONTAIN_COMMON_H */
