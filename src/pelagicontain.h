#ifndef PELAGICONTAIN_H
#define PELAGICONTAIN_H

#include "config.h"
#include "container.h"
#include <sys/types.h>

class Pelagicontain
{
public:
	Pelagicontain ();
	~Pelagicontain ();
	static int initializeConfig(struct lxc_params *ct_pars, const char *ct_base_dir, Config *config);
	int initialize (struct lxc_params &ct_pars, Config &config);
	pid_t run(int numParameters, char **parameters, struct lxc_params *ct_pars);
private:
	Container m_container;
};

#endif // PELAGICONTAIN_H
