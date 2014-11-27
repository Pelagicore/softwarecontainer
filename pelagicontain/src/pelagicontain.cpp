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
#include "paminterface.h"

Pelagicontain::Pelagicontain(const std::string &cookie) :
    m_cookie(cookie)
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
    m_container->create();
    pid_t pid = m_container->start();
    m_containerState.setValueNotify(ContainerState::PRELOADED);
    return pid;
}

void Pelagicontain::shutdownContainer()
{
    m_container->destroy();
    shutdownGateways();
    m_pamInterface->unregisterClient(m_cookie);

    m_containerState.setValueNotify(ContainerState::TERMINATED);
}


void Pelagicontain::onContainerShutdown(int pid, int exitCode)
{
    log_debug() << "Controller " << " exited with exit code " << exitCode;
    shutdownContainer();
}

void Pelagicontain::setApplicationID(const std::string &appId)
{
    m_appId = appId;

    if (m_container) {

        log_debug() << "register client " << m_cookie << " / " << m_appId;

        std::string appDirBase = m_container->root() + "/" + appId;

        // this should always be true except when unit-testing.
        if ( m_container->mountApplication(appDirBase) ) {
            m_pamInterface->registerClient(m_cookie, m_appId);
        } else {
            log_error() << "Could not set up container for application, shutting down";
            // If we are here we should have been reached over D-Bus and are
            // running in the mainloop, so we should call Pelagicontain::shutdown
            shutdown();
        }
    }

}

void Pelagicontain::launch(const std::string &appId)
{
    log_debug() << "Launch called with appId: " << appId;
    m_launching = true;
    setApplicationID(appId);
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

void Pelagicontain::update(const GatewayConfiguration &configs)
{
    log_debug() << "update called" << configs;
    setGatewayConfigs(configs);
    m_pamInterface->updateFinished(m_cookie);
    if (m_launching) {
        launchCommand(APP_BINARY);  // We launch the application with hardcoded path immediately for backward compatibility. TODO : remove
    }
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


void Pelagicontain::setContainerEnvironmentVariable(const std::string &var, const std::string &val)
{
    m_container->setEnvironmentVariable(var, val);
}

void Pelagicontain::shutdown()
{
    log_debug() << "shutdown called"; // << logging::getStackTrace();
    shutdownContainer();
}

void Pelagicontain::shutdownGateways()
{
    for (auto &gateway : m_gateways) {
        if ( !gateway->teardown() ) {
            log_warning() << "Could not tear down gateway cleanly";
        }
    }

    m_gateways.clear();
}
