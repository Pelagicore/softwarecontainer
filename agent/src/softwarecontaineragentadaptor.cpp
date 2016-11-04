#include "softwarecontaineragentadaptor.h"

softwarecontainer::SoftwareContainerAgentAdaptor::~SoftwareContainerAgentAdaptor()
{
}

softwarecontainer::SoftwareContainerAgentAdaptor::SoftwareContainerAgentAdaptor(softwarecontainer::SoftwareContainerAgent &agent) :
    m_agent(agent)
{
}

uint32_t softwarecontainer::SoftwareContainerAgentAdaptor::LaunchCommand(const int32_t &containerID, const uint32_t &userID, const std::string &commandLine, const std::string &workingDirectory, const std::string &outputFile, const std::map<std::string, std::string> &env)
{
    return m_agent.launchCommand(containerID, userID, commandLine, workingDirectory, outputFile, env,
                                 [this, containerID](pid_t pid, int exitCode) {
        ProcessStateChanged(containerID, pid, false, exitCode);
        log_info() << "ProcessStateChanged " << pid << " code " << exitCode;
    });
}

void softwarecontainer::SoftwareContainerAgentAdaptor::ShutDownContainerWithTimeout(const int32_t &containerID, const uint32_t &timeout)
{
    m_agent.shutdownContainer(containerID, timeout);
}

void softwarecontainer::SoftwareContainerAgentAdaptor::ShutDownContainer(const int32_t &containerID)
{
    m_agent.shutdownContainer(containerID);
}

std::string softwarecontainer::SoftwareContainerAgentAdaptor::BindMountFolderInContainer(const int32_t &containerID, const std::string &pathInHost, const std::string &subPathInContainer, const bool &readOnly)
{
    return m_agent.bindMountFolderInContainer(containerID, pathInHost, subPathInContainer, readOnly);
}

void softwarecontainer::SoftwareContainerAgentAdaptor::SetGatewayConfigs(const int32_t &containerID, const std::map<std::string, std::string> &configs)
{
    m_agent.setGatewayConfigs(containerID, configs);
}

bool softwarecontainer::SoftwareContainerAgentAdaptor::SetCapabilities(const int32_t &containerID, const std::vector<std::string> &capabilities)
{
    return m_agent.setCapabilities(containerID, capabilities);
}

int32_t softwarecontainer::SoftwareContainerAgentAdaptor::CreateContainer(const std::string &config)
{
    return m_agent.createContainer(config);
}

void softwarecontainer::SoftwareContainerAgentAdaptor::SetContainerName(const int32_t &containerID, const std::string &name)
{
    return m_agent.setContainerName(containerID, name);
}

void softwarecontainer::SoftwareContainerAgentAdaptor::Ping()
{
}

void softwarecontainer::SoftwareContainerAgentAdaptor::WriteToStdIn(const uint32_t &processID, const std::vector<uint8_t> &bytes)
{
    m_agent.writeToStdIn(processID, bytes);
}
