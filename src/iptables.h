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
 * Read IPTables rules from the config file, using the key 'iptables-rules',
 * finally execute these rules
 *
 * \param *params  Pointer to an initialized lxc_params struct
 * \return 0       Upon success
 * \return -EINVAL Upon missing 'iptables-rules' key
 * \return -EIO    Upon Failure to read or write files
 */
int gen_iptables_rules (struct lxc_params *params);

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
void remove_iptables_rules (struct lxc_params *params);

#endif /* IPTABLES_H */
