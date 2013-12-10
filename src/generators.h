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

/*! \brief  Generator functions
 *  \author Jonatan Pålsson (jonatan.palsson@pelagicore.com)
 *  \file   generators.h
 *
 *  Various helper functions for generating things such as network interface
 *  names, IP addresses and container names. By using these functions, unique
 *  names are ensured
 */

#ifndef GENERATORS_H
#define GENERATORS_H

#include <string>
#include "pelagicontain_common.h"

/*! \brief Generate a network interface name
 *
 * This will generate a network interface name which can be used to create new
 * virtual network interfaces
 *
 * \param ip_addr_net The network portion of the IP address for the new network
 *                    interface. Used to generate a descriptive name.
 * \return A unique network interface name
 * \return NULL upon error
 */
char *gen_net_iface_name(const char *ip_addr_net);

/*!  \brief Generate a gateway address
 *
 * Given the network portion of an IP address, this will output a suggested
 * gateway IP address.
 *
 * \return A gateway IP address
 * \return NULL upon error
 */
char *gen_gw_ip_addr (char *ip_addr_net);

/*! \brief Generate an IP address
 *
 * Calling this function will generate a new IP address. A counter is kept in
 * /tmp to minimize the risk of collissons
 *
 * \param ip_addr_net A 24 bit network portion of an IP address
 * \return A string representing an IP address
 */
std::string gen_ip_addr (const char *ip_addr_net);

/*! \brief Generate and write an LXC config
 *
 * Generate an LXC config file suitable for launching a container. This
 * configuration will be tailored to the fit the IP, gateway and interface
 * names specified in params. The configuration file can be disposed of after
 * the container has finished running. The configuration file is written to the
 * path specified in params.lxc_config_file
 *
 * \return 0       upon success
 * \return -ENOMEM upon memory allocation failure
 * \return -EINVAL upon file system failures
 */
int gen_lxc_config (struct lxc_params *params);


/*! \brief Generate a container name
 *
 * Generate a container name based on the current time in milliseconds
 *
 * \return name upon success
 * \return NULL upon failure
 */
char *gen_ct_name ();

#endif /* GENERATORS_H */
