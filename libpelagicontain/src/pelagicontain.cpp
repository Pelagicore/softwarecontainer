/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <sys/wait.h>
#include <unistd.h>
#include <glibmm.h>

#include "gateway.h"
#include "container.h"
#include "pelagicontain.h"

Pelagicontain::Pelagicontain()
{
    m_containerState = ContainerState::CREATED;
}

Pelagicontain::~Pelagicontain()
{
}

void Pelagicontain::addGateway(Gateway &gateway)
{
    gateway.setContainer(*m_container);
    m_gateways.push_back(&gateway);
}

// Preload the container. This is a non-blocking operation
pid_t Pelagicontain::preload(Container &container)
{
    m_container = &container;
    if (isError(m_container->create()))
    	return INVALID_PID;

    pid_t pid = m_container->start();
    m_containerState.setValueNotify(ContainerState::PRELOADED);
    return pid;
}

void Pelagicontain::shutdownContainer()
{
    shutdownGateways();
    m_container->destroy();

    m_containerState.setValueNotify(ContainerState::TERMINATED);
}

void Pelagicontain::shutdownContainer(unsigned int timeout)
{
    shutdownGateways();
    m_container->destroy(timeout);

    m_containerState.setValueNotify(ContainerState::TERMINATED);
}

pid_t Pelagicontain::launchCommand(const std::string &commandLine)
{
    log_debug() << "launchCommand called with commandLine: " << commandLine;
    pid_t pid = m_container->attach(commandLine);

    assert(m_mainLoopContext != nullptr);
    addProcessListener(m_connections, pid, [&](pid_t pid, int returnCode) {
                shutdown();
            }, *m_mainLoopContext);

    return pid;
}

void Pelagicontain::updateGatewayConfiguration(const GatewayConfiguration &configs)
{
    log_debug() << "updateGatewayConfiguration called" << configs;
    setGatewayConfigs(configs);
}

void Pelagicontain::setGatewayConfigs(const GatewayConfiguration &configs)
{
    // Go through the received configs and see if they match any of
    // the running gateways, if so: set their respective config

    for (auto &gateway : m_gateways) {
        std::string gatewayId = gateway->id();

        if (configs.count(gatewayId) != 0) {
            std::string config = configs.at(gatewayId);
            gateway->setConfig(config);
        }
    }

    for (auto &gateway : m_gateways) {
        gateway->activate();
    }

    m_containerState.setValueNotify(ContainerState::READY);

}

void Pelagicontain::shutdown()
{
    log_debug() << "shutdown called"; // << logging::getStackTrace();
    shutdownContainer();
}

void Pelagicontain::shutdown(unsigned int timeout)
{
    log_debug() << "shutdown called"; // << logging::getStackTrace();
    shutdownContainer(timeout);
}

void Pelagicontain::shutdownGateways()
{
    for (auto &gateway : m_gateways) {
        if (!gateway->teardown()) {
            log_warning() << "Could not tear down gateway cleanly";
        }
    }

    m_gateways.clear();
}
