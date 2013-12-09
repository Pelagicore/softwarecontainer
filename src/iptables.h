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

#ifndef IPTABLES_H
#define IPTABLES_H
/*! \brief  IPTables capabilities for Pelagicontain
 *  \author Jonatan Pålsson (joantan.palsson@pelagicore.com)
 *  \file   iptables.h
 *
 * This file contains helpers for setting up and tearing down IPTables rules
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pelagicontain_common.h"
#include "config.h"


/*! \brief Generate and execute IPTables rules
 *
 * Parse the IPTables rules from configuration and execute these rules
 *
 * \param ip_addr  The IP address of the system
 * \param rules    String containing the IPTable rules
 * \return 0       Upon success
 * \return -EINVAL Upon missing 'iptables-rules' key
 * \return -EIO    Upon Failure to read or write files
 */
int gen_iptables_rules (const char *ip_addr, const char *rules);

/*! \brief Remove IPTables rules set up for a specific network iface
 *
 * This implementation is shady. What we do here is to look at the output of
 * iptables -L, look for our own IP, and then remove all rules matching our IP
 * from the FORWARD chain. There are several problems with this:
 *	- We don't lock the iptable to ensure the list we're comparing against
 *	  matches the actual list were removing from
 *	- We don't know whether we actually added the rules ourselves, or if
 *	  someone else did.
 * in short.. this function should be re-implemented in some other way where we
 * have atomic transactions for lookup and remove, and where we're also certain
 * we are actually the originators of the rule in question.
 */
int remove_iptables_rules (const char *ip_addr);

#endif /* IPTABLES_H */
