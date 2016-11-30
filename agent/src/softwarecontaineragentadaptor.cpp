#include "softwarecontaineragentadaptor.h"

namespace softwarecontainer {

SoftwareContainerAgentAdaptor::~SoftwareContainerAgentAdaptor()
{
}

SoftwareContainerAgentAdaptor::SoftwareContainerAgentAdaptor(SoftwareContainerAgent &agent) :
    m_agent(agent)
{
}

void SoftwareContainerAgentAdaptor::Execute(
    const int32_t &containerID,
    const uint32_t &userID,
    const std::string &commandLine,
    const std::string &workingDirectory,
    const std::string &outputFile,
    const std::map<std::string, std::string> &env,
    int32_t &pid,
    bool &success)
{
    success = m_agent.execute(
        containerID,
        userID,
        commandLine,
        workingDirectory,
        outputFile,
        env,
        pid,
        [this, containerID](pid_t pid, int exitCode) {
            ProcessStateChanged(containerID, pid, false, exitCode);
            log_info() << "ProcessStateChanged " << pid << " code " << exitCode;
        }
    );
}

bool SoftwareContainerAgentAdaptor::Suspend(const int32_t &containerID)
{
    return m_agent.suspendContainer(containerID);
}

bool SoftwareContainerAgentAdaptor::Resume(const int32_t &containerID)
{
    return m_agent.resumeContainer(containerID);
}

bool SoftwareContainerAgentAdaptor::Destroy(const int32_t &containerID)
{
    return m_agent.shutdownContainer(containerID);
}

void SoftwareContainerAgentAdaptor::List(std::vector<int32_t> &containers, bool &success)
{
    success = m_agent.listContainers(containers);
    return;
}

bool SoftwareContainerAgentAdaptor::BindMount(
    const int32_t &containerID,
    const std::string &pathInHost,
    const std::string &PathInContainer,
    const bool &readOnly)
{
    return m_agent.bindMount(containerID, pathInHost, PathInContainer, readOnly);
}

bool SoftwareContainerAgentAdaptor::SetGatewayConfigs(
    const int32_t &containerID,
    const std::map<std::string, std::string> &configs)
{
    return m_agent.setGatewayConfigs(containerID, configs);
}

bool SoftwareContainerAgentAdaptor::SetCapabilities(
    const int32_t &containerID,
    const std::vector<std::string> &capabilities)
{
    return m_agent.setCapabilities(containerID, capabilities);
}

void SoftwareContainerAgentAdaptor::Create(const std::string &config,
                                           int32_t &containerID,
                                           bool &success)
{
    success = m_agent.createContainer(config, containerID);
}

} // End namespace softwarecontainer
