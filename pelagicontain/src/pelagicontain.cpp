/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <sys/wait.h>
#include <unistd.h>
#include <glibmm.h>

#include "container.h"
#include "debug.h"
#include "pelagicontain.h"

Pelagicontain::Pelagicontain(PAMAbstractInterface *pamInterface,
                             MainloopAbstractInterface *mainloopInterface,
                             ControllerAbstractInterface *controllerInterface,
                             const std::string &cookie):
    m_container(NULL),
    m_pamInterface(pamInterface),
    m_mainloopInterface(mainloopInterface),
    m_controllerInterface(controllerInterface),
    m_cookie(cookie)
{
}

Pelagicontain::~Pelagicontain()
{
    if (m_container)
    {
        delete m_container;
    }
}

void Pelagicontain::addGateway(Gateway *gateway)
{
    m_gateways.push_back(gateway);
}

/* Preload the container. This is a non-blocking operation */
pid_t Pelagicontain::preload(const std::string &containerName,
                             const std::string &containerConfig,
                             const std::string &containerRoot,
                             const std::string &containedCommand)
{
    m_container = new Container(containerName, containerConfig, containerRoot);
    Glib::SignalChildWatch cw = Glib::signal_child_watch();

    /* Get the commands to run in a separate process */
    std::vector<std::string> commands;
    commands = m_container->commands(containedCommand, m_gateways);

    std::string createCommand = commands[0];
    std::string executeCommand = commands[1];
    std::string destroyCommand = commands[2];

    log_debug(createCommand.c_str());
    system(createCommand.c_str());

    std::vector<std::string> executeCommandVec;
    executeCommandVec = Glib::shell_parse_argv(executeCommand);

    int pid;
    Glib::spawn_async_with_pipes(".",
                                 executeCommandVec,
                                 Glib::SPAWN_DO_NOT_REAP_CHILD 
                                    | Glib::SPAWN_SEARCH_PATH,
                                 sigc::slot<void>(),
                                 &pid);

    sigc::slot<void, int, int> shutdownSlot;
    shutdownSlot = sigc::bind<0>(
        sigc::mem_fun(*this, 
                      &Pelagicontain::handleControllerShutdown),
        destroyCommand /* First param to handleControllerShutdown */);
    cw.connect(shutdownSlot, pid);

    return pid;
}

void Pelagicontain::handleControllerShutdown(const std::string lxcExitCommand,
                                                   int pid,
                                                   int exitCode) {
    log_debug("Controller (pid %d) exited with code: %d. "
              "Shutting down now..", pid, exitCode);

    log_debug("Issuing: %s", lxcExitCommand.c_str());
    Glib::spawn_command_line_sync(lxcExitCommand);

    shutdownGateways();
    m_pamInterface->unregisterClient(m_cookie);

    log_debug("Queueing up main loop termination");
    Glib::signal_idle().connect(
        sigc::mem_fun(*this, &Pelagicontain::killMainLoop));
}

void Pelagicontain::launch(const std::string &appId) {
    m_appId = appId;
    if (m_container)
    {
        // this should always be true except when unit-testing.
        m_container->setApplication(appId);
    }
    m_pamInterface->registerClient(m_cookie, m_appId);
}

void Pelagicontain::update(const std::map<std::string, std::string> &configs)
{
    setGatewayConfigs(configs);

    m_pamInterface->updateFinished(m_cookie);

    activateGateways();

    m_controllerInterface->startApp();
}

void Pelagicontain::setGatewayConfigs(const std::map<std::string, std::string> &configs)
{
    /* Go through the received configs and see if they match any of
     * the running gateways, if so: set their respective config
     */
    std::string config;
    std::string gatewayId;

    for (std::vector<Gateway *>::iterator gateway = m_gateways.begin();
         gateway != m_gateways.end(); ++gateway) {
        gatewayId = (*gateway)->id();
        if (configs.count(gatewayId) != 0) {
            config = configs.at(gatewayId);
            (*gateway)->setConfig(config);
        }
    }
}

void Pelagicontain::activateGateways()
{
    for (std::vector<Gateway *>::iterator gateway = m_gateways.begin();
         gateway != m_gateways.end(); ++gateway) {
        (*gateway)->activate();
    }
}

void Pelagicontain::setContainerEnvironmentVariable(const std::string &var, const std::string &val)
{
    m_controllerInterface->setEnvironmentVariable(var, val);
}

void Pelagicontain::shutdown()
{
    /* Tell Controller to shut down the app
     * Controller will exit when the app has shut down and then
     * lxc-execute will return and lxc-destroy be run (see above
     * code in the forked child)
     */
    m_controllerInterface->shutdown();
}

void Pelagicontain::shutdownGateways()
{
    for (std::vector<Gateway *>::iterator gateway = m_gateways.begin();
         gateway != m_gateways.end(); ++gateway)
    {
        if (!(*gateway)->teardown())
            log_warning("Could not teardown gateway cleanly");
        delete (*gateway);
    }

    m_gateways.clear();
}
