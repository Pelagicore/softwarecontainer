
/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */


#pragma once

#include <glibmm.h>
#include "softwarecontainer-common.h"

namespace com {
namespace pelagicore {
class SoftwareContainerAgent_proxy;
}
}

namespace softwarecontainer {

class AgentPrivateData;
class AgentContainer;
class AgentCommand;

class Agent
{

public:
    Agent(Glib::RefPtr<Glib::MainContext> mainLoopContext);

    ~Agent();

    /**
     * Set the main loop
     */
    void setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext);

    pid_t startProcess(AgentCommand &command, std::string &cmdLine, uid_t userID, const std::string &workingDirectory,
                const std::string &outputFile,
                EnvironmentVariables env);

    void shutDown(ContainerID containerID);
    void shutDown(ContainerID containerID, unsigned int timeout);

    std::string bindMountFolderInContainer(ContainerID containerID, const std::string &src, const std::string &dst, bool readonly);

    void setGatewayConfigs(ContainerID containerID, const GatewayConfiguration &config);

    ReturnCode writeToStdIn(pid_t pid, const void *data, size_t length);

    ReturnCode createContainer(const std::string &idPrefix, ContainerID &containerID, const std::string &config);

    ReturnCode setContainerName(ContainerID containerID, const std::string &name);

    AgentContainer *getContainer(ContainerID containerID)
    {
        return (m_containers.count(containerID) != 0) ? m_containers[containerID] : nullptr;
    }

    AgentCommand *getCommand(pid_t pid)
    {
        return (m_commands.count(pid) != 0) ? m_commands[pid] : nullptr;
    }

private:
    com::pelagicore::SoftwareContainerAgent_proxy &getProxy();

    Glib::RefPtr<Glib::MainContext> m_ml;
    AgentPrivateData *m_p = nullptr;

    std::map<ContainerID, AgentContainer *> m_containers;
    std::map<pid_t, AgentCommand *> m_commands;

};

class AgentContainer
{

public:
    AgentContainer(Agent &agent) :
        m_agent(agent)
    {
    }

    ~AgentContainer()
    {
        // We stop the container on destruction
        shutdown();
    }

    ReturnCode init()
    {
        std::string config;
        if (m_enableWriteBuffer)
            config = "{enableWriteBuffer: \"1\"}";
        else
            config = "{enableWriteBuffer: \"0\"}";
        auto ret = m_agent.createContainer(m_name, m_containerID, config);
        if (ret == ReturnCode::SUCCESS) {
            m_containerState = ContainerState::PRELOADED;
        }

        return ret;
    }

    void setName(const std::string &name)
    {
        m_agent.setContainerName(m_containerID, name);
    }

    void shutdown()
    {
        m_agent.shutDown(getContainerID());
    }

    void shutdown(unsigned int timeout)
    {
        m_agent.shutDown(getContainerID(), timeout);
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
    std::string m_name;
    bool m_enableWriteBuffer;
};

class AgentCommand
{

public:
    enum class ProcessState
    {
        STOPPED,
        RUNNING,
        TERMINATED
    };

    AgentCommand(AgentContainer &container, std::string cmd) :
        m_container(container), m_cmdLine(cmd)
    {
    }

    ReturnCode setWorkingDirectory(const std::string &directory)
    {
        m_workingDirectory = directory;
        return ReturnCode::SUCCESS;
    }

    ReturnCode setOutputFile(const std::string &file)
    {
        m_outputFile = file;
        return ReturnCode::SUCCESS;
    }

    ReturnCode setUserID(uid_t userID)
    {
        m_userID = userID;
        return ReturnCode::SUCCESS;
    }

    void start()
    {
        m_pid = m_container.getAgent().startProcess(*this, m_cmdLine, m_userID, m_workingDirectory, m_outputFile, m_envVariables);
    }

    void addEnvironnmentVariable(const std::string &key, const std::string &value)
    {
        m_envVariables[key] = value;
    }

    void setEnvironnmentVariables(const EnvironmentVariables &variables)
    {
        m_envVariables = variables;
    }

    pid_t pid() const
    {
        return m_pid;
    }

    void captureStdin()
    {
        //		assert(false);
    }

    ReturnCode writeToStdIn(const void *data, size_t length)
    {
        return m_container.getAgent().writeToStdIn(m_pid, data, length);
    }

    ObservableProperty<ProcessState> &getState()
    {
        return m_processState;
    }

    void setState(ProcessState state)
    {
        m_processState.setValueNotify(state);
    }

    AgentContainer &getContainer()
    {
        return m_container;
    }

private:
    ObservableWritableProperty<ProcessState> m_processState = ProcessState::STOPPED;
    AgentContainer &m_container;
    pid_t m_pid = INVALID_PID;
    std::string m_cmdLine;
    std::string m_workingDirectory;
    std::string m_outputFile;
    EnvironmentVariables m_envVariables;
    SignalConnectionsHandler m_connections;
    uid_t m_userID = ROOT_UID;

};

}
