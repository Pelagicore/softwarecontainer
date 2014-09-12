/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <sys/wait.h>
#include <unistd.h>
#include <glibmm.h>

#include "container.h"
#include "pelagicontain.h"

Pelagicontain::Pelagicontain(ControllerInterface *controllerInterface,
                             const std::string &cookie):
    m_container(nullptr),
    m_controllerInterface(controllerInterface),
    m_cookie(cookie)
{
	m_containerState = ContainerState::CREATED;
}

Pelagicontain::~Pelagicontain()
{
}

void Pelagicontain::addGateway(Gateway& gateway)
{
    m_gateways.push_back(&gateway);
}

// Preload the container. This is a non-blocking operation
pid_t Pelagicontain::preload(Container *container)
{
    m_container = container;

    m_container->create();

    pid_t pid = m_container->start();

    // The pid might not valid if there was an error spawning. We should only
    // connect the watcher if the spawning went well.
    if (pid) {
    	addProcessListener(pid, sigc::mem_fun(this, &Pelagicontain::onControllerShutdown));
    }

    return pid;
}

ReturnCode Pelagicontain::establishConnection()
{
    return m_controllerInterface->initialize();
}

void Pelagicontain::shutdownContainer()
{
	m_container->destroy();
    shutdownGateways();
    m_pamInterface->unregisterClient(m_cookie);

    m_containerState.setValueNotify(ContainerState::TERMINATED);
}


void Pelagicontain::onControllerShutdown(int pid, int exitCode)
{
    log_debug() << "Controller " << " exited with exit code " << exitCode;
    shutdownContainer();
}

void Pelagicontain::launch(const std::string &appId)
{
    log_debug() << "Launch called with appId: " << appId;
    m_launching = true;
    m_appId = appId;
    if (m_container) {
        // this should always be true except when unit-testing.
    	// TODO : rename setApplication(), which is not a setter
        if (m_container->setApplication(appId)) {
            m_pamInterface->registerClient(m_cookie, m_appId);
        } else {
            log_error() << "Could not set up container for application, shutting down";
            // If we are here we should have been reached over D-Bus and are
            // running in the mainloop, so we should call Pelagicontain::shutdown
            shutdown();
        }
    }
}

void Pelagicontain::launchCommand(const std::string &commandLine)
{
    log_debug() << "launchCommand called with commandLine: " << commandLine;
    pid_t pid = m_container->attach(commandLine);

    addProcessListener(pid, [&](pid_t pid, int returnCode) {
    	shutdown();
    });
}

void Pelagicontain::update(const GatewayConfiguration &configs)
{
    log_debug() << "update called";
    setGatewayConfigs(configs);

    m_pamInterface->updateFinished(m_cookie);

    activateGateways();

    // We should only start the app if we have ended up here because launch was
    // called and the app has not been started previously.
    if (m_launching && !m_controllerInterface->hasBeenStarted()) {
    	launchCommand(APP_BINARY);
    }
}

void Pelagicontain::setGatewayConfigs(const GatewayConfiguration &configs)
{
    // Go through the received configs and see if they match any of
    // the running gateways, if so: set their respective config

    for (auto& gateway : m_gateways)
    {
    	std::string gatewayId = gateway->id();
        if (configs.count(gatewayId) != 0) {
            std::string config = configs.at(gatewayId);
            gateway->setConfig(config);
        }
    }
}

void Pelagicontain::activateGateways()
{
    for (auto& gateway : m_gateways)
        gateway->activate();
}

void Pelagicontain::setContainerEnvironmentVariable(const std::string &var, const std::string &val)
{
	m_container->setEnvironmentVariable(var,val);
}

void Pelagicontain::shutdown()
{
    log_debug() << "shutdown called"; // << logging::getStackTrace();
    shutdownContainer();
}

void Pelagicontain::shutdownGateways()
{
    for (auto& gateway : m_gateways)
    {
        if (!gateway->teardown()) {
            log_warning() << "Could not tear down gateway cleanly";
        }
    }

    m_gateways.clear();
}
