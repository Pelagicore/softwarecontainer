/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PELAGICONTAINCOMMON_H
#define PELAGICONTAINCOMMON_H

#include <string>

/*! \brief  Common code for Pelagicontain
 *  \file pelagicontaincommon.h
 *
 * Here is common code, such as shared data structures
 */
struct lxc_params {

	/* networking */
	/*! IPv4 address for this container */
	std::string ip_addr;

	/*! IPv4 address for the default GW of this container */
	char *gw_addr; 

	/*! External interface name for the networking interface of the
	 * container */
	std::string net_iface_name; 


	/* traffic control */
	/*! Rate limit for the networking interface, Kbps and Mbps suffixes are
	 * supported, for instance: 500Kbps */
	char *tc_rate; 

	/* pulse audio */
	/*! Path to the Pulse Audio socket as seen from the host system */
	char pulse_socket[1024]; 


	/* D-Bus */
	/*! Path to the D-Bus session proxy socket as seen from the host */
	char session_proxy_socket[1024];

	/*! Path to the D-Bus system proxy socket as seen from the host */
	char system_proxy_socket[1024];


	/* LXC general */
	/*! Unique name of the container, used to identify it in LXC */
	std::string container_name;

	/*! Path to the LXC system config, as seen from the host */
	char *lxc_system_cfg;

	/*! lxc_params#ct_dir  + "/config/" */
	char ct_conf_dir[1024];

	/*! lxc_params#ct_dir + "/rootfs/" */
	char ct_root_dir[1024];

	/* general */
	/*! Path to Pelagicontain config file, as seen from host */
	char main_cfg_file[1024];
};

#endif /* PELAGICONTAINCOMMON_H */
