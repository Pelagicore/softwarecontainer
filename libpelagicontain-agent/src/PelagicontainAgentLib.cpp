#include "PelagicontainAgentLib.h"
#include "PelagicontainAgentLib_dbuscpp_proxy.h"

#include "pelagicore-DBusCpp.h"

#include <memory>

namespace pelagicontain {

class Agent;

LOG_DECLARE_DEFAULT_CONTEXT(defaultContext, "SCAL", "Software container agent library");

struct AgentPrivateData
{
    class PelagicontainAgentProxy :
        public com::pelagicore::PelagicontainAgent_proxy
    {

public:
        PelagicontainAgentProxy(Agent &agent) :
            m_agent(agent)
        {
        }

        void ProcessStateChanged(const uint32_t &containerID, const uint32_t &pid, const bool &isRunning,
                    const uint32_t &exitCode) override;

private:
        Agent &m_agent;
    };

    AgentPrivateData(Glib::RefPtr<Glib::MainContext> mainLoopContext, Agent &agent) :
        m_gLibDBusCppFactory(mainLoopContext)
    {
        try {
            m_proxy = m_gLibDBusCppFactory.registerProxy<PelagicontainAgentProxy>(
                        m_gLibDBusCppFactory.getSystemBusConnection(), AGENT_OBJECT_PATH, AGENT_BUS_NAME, agent);
            m_proxy->Ping();
        } catch (DBus::Error &error) {
            m_proxy = m_gLibDBusCppFactory.registerProxy<PelagicontainAgentProxy>(
                        m_gLibDBusCppFactory.getSessionBusConnection(), AGENT_OBJECT_PATH, AGENT_BUS_NAME, agent);
            m_proxy->Ping();
        }
    }

    std::unique_ptr<PelagicontainAgentProxy> m_proxy;

private:
    pelagicore::GLibDBusCppFactory m_gLibDBusCppFactory;
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

com::pelagicore::PelagicontainAgent_proxy &Agent::getProxy()
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

ReturnCode Agent::createContainer(const std::string &idPrefix, ContainerID &containerID)
{
    containerID = getProxy().CreateContainer(idPrefix);
    return (containerID != INVALID_CONTAINER_ID) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
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

void AgentPrivateData::PelagicontainAgentProxy::ProcessStateChanged(const uint32_t &containerID, const uint32_t &pid,
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
