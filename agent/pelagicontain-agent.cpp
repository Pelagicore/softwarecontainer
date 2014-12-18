/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

/**
 * That file contains an executable which register a DBUS service which lets client applications manipulate LXC containers.
 * This executable needs to run by the root user since that is required by LXC.
 */

#include <glibmm.h>
#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

#include "CommandLineParser.h"
#include "pelagicore-log.h"

#include "UNIXSignalGlibHandler.h"

#include "pelagicore-DBusCpp.h"

#include "PelagicontainAgent_dbuscpp_adaptor.h"

#include "pelagicontain-lib.h"

LOG_DEFINE_APP_IDS("PELA", "Pelagicontain agent");

namespace pelagicontain {

LOG_DECLARE_DEFAULT_CONTEXT(PAM_DefaultLogContext, "MAIN", "Main context");

class PelagicontainAgent
{

public:
    PelagicontainAgent(Glib::RefPtr<Glib::MainContext> mainLoopContext, int preloadCount) :
        m_mainLoopContext(mainLoopContext)
    {
        m_preloadCount = preloadCount;

        triggerPreload();
    }

    void triggerPreload()
    {
        log_debug() << "triggerPreload " << m_preloadCount - m_preloadedContainers.size();
        while (m_preloadedContainers.size() != m_preloadCount) {
            auto container = new PelagicontainLib();
            container->preload();
            m_preloadedContainers.push_back(container);
        }
    }

    bool checkContainer(ContainerID containerID, PelagicontainLib * &container)
    {
        bool valid = ( ( containerID < m_containers.size() ) && (m_containers[containerID] != nullptr) );
        if (valid) {
            container = m_containers[containerID];
        } else {
            log_error() << "Invalid container ID " << containerID;
        }

        return valid;
    }

    ContainerID createContainer()
    {

        PelagicontainLib *container;
        if (m_preloadedContainers.size() != 0) {
            container = m_preloadedContainers[0];
            m_preloadedContainers.erase( m_preloadedContainers.begin() );
        } else {
            container = new PelagicontainLib();
        }

        m_containers.push_back(container);
        auto id = m_containers.size() - 1;
        log_debug() << "Created container with ID :" << id;
        container->setMainLoopContext(m_mainLoopContext);
        container->init();

        triggerPreload();

        return id;
    }

    pid_t launchCommand(ContainerID containerID, const std::string &cmdLine, const std::string &workingDirectory,
                EnvironmentVariables env)
    {
        PelagicontainLib *container = nullptr;
        if ( checkContainer(containerID, container) ) {
            return container->getContainer().attach(cmdLine, env, workingDirectory);
        }
        return -1;
    }

    void shutdownContainer(ContainerID containerID)
    {
        PelagicontainLib *container = nullptr;
        if ( checkContainer(containerID, container) ) {
            container->shutdown();
        }
    }

    std::string bindMountFolderInContainer(const uint32_t containerID, const std::string &pathInHost,
                const std::string &subPathInContainer, bool readOnly)
    {
        PelagicontainLib *container = nullptr;
        if ( checkContainer(containerID, container) ) {
            return container->getContainer().bindMountFolderInContainer(pathInHost, subPathInContainer, readOnly);
        }
        return "";
    }

    void setGatewayConfigs(const uint32_t &containerID, const std::map<std::string, std::string> &configs)
    {
        PelagicontainLib *container = nullptr;
        if ( checkContainer(containerID, container) ) {
            container->getPelagicontain().updateGatewayConfiguration(configs);
        }
    }

private:
    std::vector<PelagicontainLib *> m_containers;
    std::vector<PelagicontainLib *> m_preloadedContainers;
    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    size_t m_preloadCount;

};

class PelagicontainAgentAdaptor :
    public com::pelagicore::PelagicontainAgent_adaptor
{

public:
    ~PelagicontainAgentAdaptor()
    {
    }

    PelagicontainAgentAdaptor(PelagicontainAgent &agent) :
        m_agent(agent)
    {
    }

    uint32_t LaunchCommand(const uint32_t &containerID, const std::string &commandLine, const std::string &workingDirectory,
                const std::map<std::string, std::string> &env)
    {
        return m_agent.launchCommand(containerID, commandLine, workingDirectory, env);
    }

    void ShutDownContainer(const uint32_t &containerID) override
    {
        m_agent.shutdownContainer(containerID);
    }

    std::string BindMountFolderInContainer(const uint32_t &containerID, const std::string &pathInHost,
                const std::string &subPathInContainer, const bool &readOnly) override
    {
        return m_agent.bindMountFolderInContainer(containerID, pathInHost, subPathInContainer, readOnly);
    }

    void SetGatewayConfigs(const uint32_t &containerID, const std::map<std::string, std::string> &configs) override
    {
        m_agent.setGatewayConfigs(containerID, configs);
    }

    uint32_t CreateContainer() override
    {
        return m_agent.createContainer();
    }

    PelagicontainAgent &m_agent;

};

}

int main(int argc, char * *argv)
{

    log_debug() << "Starting pelagicontain agent";

    pelagicore::CommandLineParser commandLineParser("Pelagicontain agent", "", PACKAGE_VERSION, "");

    int preloadCount = 3;
    commandLineParser.addOption(preloadCount, "preload", 'p', "Number of containers to preload");

    if ( commandLineParser.parse(argc, argv) ) {
        exit(1);
    }

    auto mainContext = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create(mainContext);

    pelagicore::GLibDBusCppFactory glibDBusFactory(mainContext);
    glibDBusFactory.getConnection().request_name(AGENT_BUS_NAME);

    PelagicontainAgent agent(mainContext, preloadCount);

    auto pp = glibDBusFactory.registerAdapter<PelagicontainAgentAdaptor>(AGENT_OBJECT_PATH, agent);

    // Register signalHandler with signals
    std::vector<int> signals = {SIGINT, SIGTERM};
    pelagicore::UNIXSignalGlibHandler handler( signals, [&] (int signum) {
                log_debug() << "caught signal " << signum;
                ml->quit();
            }, ml->get_context()->gobj() );

    ml->run();

    log_debug() << "Exiting pelagicontain agent";

    return 0;
}
