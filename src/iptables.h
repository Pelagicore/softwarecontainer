#ifndef IPTABLES_H
#define IPTABLES_H
#include "pelagicontain_common.h"

char *gen_iptables_rules (struct lxc_params *params);
void remove_iptables_rules (struct lxc_params *params);

#endif /* IPTABLES_H */
