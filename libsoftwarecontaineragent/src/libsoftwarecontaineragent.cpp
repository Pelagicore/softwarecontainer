
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


#include "libsoftwarecontaineragent.h"

#include <glibmm.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    #include "SoftwareContainerAgentLib_dbuscpp_proxy.h"
    #include <dbus-c++/dbus.h>
    #include <dbus-c++/glib-integration.h>
#pragma GCC diagnostic pop

#include <memory>

namespace softwarecontainer {

class Agent;

LOG_DECLARE_DEFAULT_CONTEXT(defaultContext, "SCAL", "Software container agent library");


struct AgentPrivateData
{
    class SoftwareContainerAgentProxy : public com::pelagicore::SoftwareContainerAgent_proxy
    {

public:
        SoftwareContainerAgentProxy(Agent &agent) :
            m_agent(agent)
        {
        }

        void ProcessStateChanged(const int32_t &containerID, const uint32_t &pid, const bool &isRunning,
                    const uint32_t &exitCode) override;

private:
        Agent &m_agent;
    };

    // Utility class for DBus Proxies
    class DBusCppProxy : public SoftwareContainerAgentProxy, public DBus::IntrospectableProxy, public DBus::ObjectProxy {
        public:
            DBusCppProxy(DBus::Connection& connection, const std::string& objectPath, const std::string& busname, Agent &agent) :
                SoftwareContainerAgentProxy(agent), DBus::ObjectProxy(connection, objectPath, busname.c_str())
            {
            }
    };

    AgentPrivateData(Glib::RefPtr<Glib::MainContext> mainLoopContext, Agent &agent)
    {
        try {
            m_conn = std::unique_ptr<DBus::Connection>(new DBus::Connection(DBus::Connection::SystemBus()));
            m_proxy = std::unique_ptr<SoftwareContainerAgentProxy>(new DBusCppProxy(*m_conn, AGENT_OBJECT_PATH, AGENT_BUS_NAME, agent));
            m_proxy->Ping();
        } catch (DBus::Error &error) {
            m_conn = std::unique_ptr<DBus::Connection>(new DBus::Connection(DBus::Connection::SessionBus()));
            m_proxy = std::unique_ptr<SoftwareContainerAgentProxy>(new DBusCppProxy(*m_conn, AGENT_OBJECT_PATH, AGENT_BUS_NAME, agent));
            m_proxy->Ping();
        }
    }

    std::unique_ptr<SoftwareContainerAgentProxy> m_proxy;
private:
    std::unique_ptr<DBus::Connection> m_conn;
};


Agent::Agent(Glib::RefPtr<Glib::MainContext> mainLoopContext)
{
    setMainLoopContext(mainLoopContext);
    m_p = new AgentPrivateData(mainLoopContext, *this);
}

Agent::~Agent()
{
    delete m_p;
}

com::pelagicore::SoftwareContainerAgent_proxy &Agent::getProxy()
{
    return *m_p->m_proxy.get();
}

void Agent::setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext)
{
    m_ml = mainLoopContext;
}

pid_t Agent::startProcess(AgentCommand &command, std::string &cmdLine, uid_t userID, const std::string &workingDirectory,
            const std::string &outputFile,
            EnvironmentVariables env)
{
    auto pid = getProxy().LaunchCommand(
                command.getContainer().getContainerID(), userID, cmdLine, workingDirectory, outputFile, env);
    m_commands[pid] = &command;
    return pid;
}

void Agent::shutDown(ContainerID containerID)
{
    getProxy().ShutDownContainer(containerID);
}

void Agent::shutDown(ContainerID containerID, unsigned int timeout)
{
    getProxy().ShutDownContainerWithTimeout(containerID, timeout);
}

std::string Agent::bindMountFolderInContainer(ContainerID containerID, const std::string &src, const std::string &dst,
            bool readonly)
{
    return getProxy().BindMountFolderInContainer(containerID, src, dst, readonly);
}

void Agent::setGatewayConfigs(ContainerID containerID, const GatewayConfiguration &config)
{
    getProxy().SetGatewayConfigs(containerID, config);
}

ReturnCode Agent::createContainer(ContainerID &containerID, const std::string &config)
{
    containerID = getProxy().CreateContainer(config);
    return bool2ReturnCode(containerID != INVALID_CONTAINER_ID);
}

ReturnCode Agent::setContainerName(ContainerID containerID, const std::string &name)
{
    getProxy().SetContainerName(containerID, name);
    return ReturnCode::SUCCESS;
}

ReturnCode Agent::writeToStdIn(pid_t pid, const void *data, size_t length)
{
    auto c = static_cast<const uint8_t *>(data);
    std::vector<uint8_t> dataAsVector(c, c + length);
    getProxy().WriteToStdIn(pid, dataAsVector);
    return ReturnCode::SUCCESS;
}

void AgentPrivateData::SoftwareContainerAgentProxy::ProcessStateChanged(const int32_t &containerID, const uint32_t &pid,
            const bool &isRunning,
            const uint32_t &exitCode)
{
    auto command = m_agent.getCommand(pid);
    if (command != nullptr) {
        command->setState(isRunning ? AgentCommand::ProcessState::RUNNING : AgentCommand::ProcessState::TERMINATED);
    } else {
        log_warning() << "Unknown pid : " << pid;
    }
}

}
