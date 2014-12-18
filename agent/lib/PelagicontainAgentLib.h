#pragma once

#include <glibmm.h>
#include "pelagicontain-common.h"

namespace com {
namespace pelagicore {
class PelagicontainAgent_proxy;
}
}

namespace pelagicontain {

class AgentPrivateData;

class Agent
{

public:
    Agent(Glib::RefPtr<Glib::MainContext> mainLoopContext);

    ~Agent();

    /**
     * Set the main loop
     */
    void setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext);

    pid_t startProcess(ContainerID container, std::string &cmdLine, const std::string &workingDirectory, EnvironmentVariables env);

    void shutDown(ContainerID containerID);

    std::string bindMountFolderInContainer(ContainerID containerID, const std::string &src, const std::string &dst, bool readonly);

    void setGatewayConfigs(ContainerID containerID, const GatewayConfiguration &config);

    ReturnCode createContainer(ContainerID &containerID);

private:
    com::pelagicore::PelagicontainAgent_proxy &getProxy();

    Glib::RefPtr<Glib::MainContext> m_ml;
    AgentPrivateData *m_p = nullptr;

};

class AgentContainer
{

public:
    AgentContainer(Agent &agent) :
        m_agent(agent)
    {
    }

    ReturnCode init()
    {
        auto ret = m_agent.createContainer(m_containerID);
        if (ret == ReturnCode::SUCCESS) {
            m_containerState = ContainerState::PRELOADED;
        }

        return ret;
    }

    void shutdown()
    {
        m_agent.shutDown( getContainerID() );
    }

    bool isInitialized()
    {
        return (m_containerState == ContainerState::PRELOADED);
    }

    ObservableProperty<ContainerState> &getContainerState()
    {
        return m_containerState;
    }

    std::string bindMountFolderInContainer(const std::string &src, const std::string &dst, bool readonly = true)
    {
        return m_agent.bindMountFolderInContainer(getContainerID(), src, dst, readonly);
    }

    void setGatewayConfigs(const GatewayConfiguration &config)
    {
        m_agent.setGatewayConfigs(getContainerID(), config);
        m_containerState = ContainerState::READY;
    }

    Agent &getAgent()
    {
        return m_agent;
    }

    ContainerID getContainerID() const
    {
        return m_containerID;
    }

private:
    Agent &m_agent;
    ObservableWritableProperty<ContainerState> m_containerState = ContainerState::CREATED;
    ContainerID m_containerID;

};

class AgentCommand
{

public:
    AgentCommand(AgentContainer &container, std::string cmd) :
        m_container(container), m_cmdLine(cmd)
    {
    }

    ReturnCode setWorkingDirectory(const std::string &directory)
    {
        m_workingDirectory = directory;
        return ReturnCode::SUCCESS;
    }

    void start()
    {
        m_pid = m_container.getAgent().startProcess(m_container.getContainerID(), m_cmdLine, m_workingDirectory, m_envVariables);
    }

    void setEnvironnmentVariable(const std::string &key, const std::string &value)
    {
        m_envVariables[key] = value;
    }

    pid_t pid() const
    {
        return m_pid;
    }

    void captureStdin()
    {
        //		assert(false);
    }

    size_t write(const void *data, size_t length)
    {
        return length;
    }

private:
    AgentContainer &m_container;
    pid_t m_pid = INVALID_PID;
    std::string m_cmdLine;
    std::string m_workingDirectory;
    EnvironmentVariables m_envVariables;

};

}
