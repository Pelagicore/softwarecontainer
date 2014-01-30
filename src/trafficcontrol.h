/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef TRAFFICCONTROL_H
#define TRAFFICCONTROL_H

#include "pelagicontaincommon.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "ifaddrs.h"
#include "string.h"
#include "errno.h"
#include "debug.h"

/*! \brief Traffic control functionality
 *  \file trafficcontrol.h
 *
 *  Traffic control functionality for Pelagicontain. Allows interfacing with
 *  the linux tc tool, allows things such as bandwidth limiting
 */

/*! \brief Place a bandwidth limitation on a network interface
 *
 * Limit the incoming and outgoing bandwidth on a network interface. The limits
 * are specified using params.tc_rate. The interface is specified using
 * params.net_iface_name.
 *
 * This function forks an observing process in order to detect the availability
 * of the network interface.
 *
 * \param net_iface_name Network interface name
 * \param tc_rate        TC rate 
 * \return 0       Upon success
 * \return -EINVAL Upon failure
 */
int limit_iface (const char *net_iface_name, const char *tc_rate);


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
