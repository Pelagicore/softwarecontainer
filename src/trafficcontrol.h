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

/*! \brief Traffic control functionality
 *  \author Jonatan Pålsson (jonatan.palsson@pelagicore.com)
 *  \file trafficcontrol.h
 *
 *  Traffic control functionality for Pelagicontain. Allows interfacing with
 *  the linux tc tool, allows things such as bandwidth limiting
 */

#ifndef TRAFFICCONTROL_H
#define TRAFFICCONTROL_H

#include "pelagicontain_common.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "ifaddrs.h"
#include "string.h"
#include "errno.h"
#include "debug.h"

/*! \brief Place a bandwidth limitation on a network interface
 *
 * Limit the incoming and outgoing bandwidth on a network interface. The limits
 * are specified using params.tc_rate. The interface is specified using
 * params.net_iface_name.
 *
 * This function forks an observing process in order to detect the availability
 * of the network interface.
 *
 * \param params   An initialized lxc_params struct, specifically tc_rate and
 *                 net_iface_name must be set
 * \return 0       Upon success
 * \return -EINVAL Upon failure
 */
int limit_iface (struct lxc_params *params);


/*! \brief Remove the limits set by limit_iface()
 *
 * This will remove any limits set by limit_iface(). This call assumes the
 * network interface is available, and will not block until it can be obtained.
 *
 * \param  iface   name of interface
 * \return 0       upon success
 * \return -EINVAL upon failure to remove limit, or failure to allocate memory
 */
int clear_iface_limits (char *iface);

#endif /* TRAFFICCONTROL_H */
