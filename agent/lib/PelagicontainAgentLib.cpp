#include "PelagicontainAgentLib.h"
#include "PelagicontainAgent_dbuscpp_proxy.h"

#include "pelagicore-DBusCpp.h"

#include <memory>

namespace pelagicontain {

struct AgentPrivateData
{

    AgentPrivateData(Glib::RefPtr<Glib::MainContext> mainLoopContext) :
        m_gLibDBusCppFactory(mainLoopContext)
    {
        m_proxy = m_gLibDBusCppFactory.registerProxy<com::pelagicore::PelagicontainAgent_proxy>(AGENT_OBJECT_PATH, AGENT_BUS_NAME);
    }

    std::unique_ptr<com::pelagicore::PelagicontainAgent_proxy> m_proxy;

    pelagicore::GLibDBusCppFactory m_gLibDBusCppFactory;
};

Agent::Agent(Glib::RefPtr<Glib::MainContext> mainLoopContext)
{
    setMainLoopContext(mainLoopContext);
    m_p = new AgentPrivateData(mainLoopContext);
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

pid_t Agent::startProcess(ContainerID containerID, std::string &cmdLine, const std::string &workingDirectory,
            EnvironmentVariables env)
{
    return getProxy().LaunchCommand(containerID, cmdLine, workingDirectory, env);
}

void Agent::shutDown(ContainerID containerID)
{
    getProxy().ShutDownContainer(containerID);
    assert(false);
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

ReturnCode Agent::createContainer(ContainerID &containerID)
{
    containerID = getProxy().CreateContainer();
    return (containerID != INVALID_CONTAINER_ID) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}

}
