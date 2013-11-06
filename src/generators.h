#ifndef GENERATORS_H
#define GENERATORS_H

#include "stdio.h"
#include "stdlib.h"
#include "pelagicontain_common.h"
#include "debug.h"
#include "string.h"

char *gen_net_iface_name (char *ip_addr_net);

char *gen_gw_ip_addr (char *ip_addr_net);

char *gen_ip_addr (char *ip_addr_net);

char *gen_lxc_config (struct lxc_params *params);

char *gen_ct_name ();

#endif /* GENERATORS_H */
