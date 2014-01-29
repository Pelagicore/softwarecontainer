
#ifndef PELAGICONTAIN_H
#define PELAGICONTAIN_H

#include <sys/types.h>

#include "config.h"
#include "container.h"
#include "paminterface.h"
#include "controllerinterface.h"

class Pelagicontain {
public:
	Pelagicontain(const PAMInterface &pamInterface);
	~Pelagicontain();
	static int initializeConfig(struct lxc_params *ct_pars, const char *ct_base_dir, Config *config);
	int initialize(struct lxc_params &ct_pars, Config &config);
	pid_t run(int numParameters, char **parameters, struct lxc_params *ct_pars);
	void launch(const std::string &appId);
	void update(const std::map<std::string, std::string> &configs);
	void shutdown();

private:
	void setGatewayConfigs(const std::map<std::string, std::string> &configs);
	void activateGateways();
	void shutdownGateways();

	ControllerInterface m_controller;
	Container m_container;
	PAMInterface m_pamInterface;
	std::vector<Gateway *> m_gateways;
	std::string m_appId;
};

#endif // PELAGICONTAIN_H
