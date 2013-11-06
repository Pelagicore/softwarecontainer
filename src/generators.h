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

#ifndef GENERATORS_H
#define GENERATORS_H

#include "stdio.h"
#include "stdlib.h"
#include "pelagicontain_common.h"
#include "debug.h"
#include "string.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/file.h"
#include "fcntl.h"

char *gen_net_iface_name (char *ip_addr_net);

char *gen_gw_ip_addr (char *ip_addr_net);

/*! \brief Generate an IP address
 *
 * Calling this function will generate a new IP address. A counter is kept in
 * /tmp to minimize the risk of collissons
 *
 * \param *ip_addr_net A 24 bit network portion of an IP address
 * \return A string representing an IP address
 */
char *gen_ip_addr (char *ip_addr_net);

char *gen_lxc_config (struct lxc_params *params);

char *gen_ct_name ();

#endif /* GENERATORS_H */
