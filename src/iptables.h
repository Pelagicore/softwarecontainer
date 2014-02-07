/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef IPTABLES_H
#define IPTABLES_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pelagicontaincommon.h"
#include "config.h"

/*! \brief  IPTables capabilities for Pelagicontain
 *  \file   iptables.h
 *
 * This file contains helpers for setting up and tearing down IPTables rules
 */
class IpTables
{
public:
	/*! Generate and execute IPTables rules
	*
	* Parse the IPTables rules from configuration and execute these rules
	*
	* \param ip_addr  The IP address of the system
	* \param rules    String containing the IPTable rules
	*/
	IpTables(const char *ip_addr, const char *rules);

	/*! Remove IPTables rules set up for a specific network iface
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
	~IpTables();

private:
	const char *m_ip;
};


#endif /* IPTABLES_H */
