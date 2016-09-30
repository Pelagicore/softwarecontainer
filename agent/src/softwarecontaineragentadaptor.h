#ifndef SOFTWARECONTAINERAGENTADAPTOR_H
#define SOFTWARECONTAINERAGENTADAPTOR_H

#include "softwarecontaineragent.h"

namespace softwarecontainer {

class SoftwareContainerAgentAdaptor :
    public com::pelagicore::SoftwareContainerAgent_adaptor
{
    LOG_DECLARE_CLASS_CONTEXT("SCAA", "SoftwareContainerAgentAdaptor");

public:
    virtual ~SoftwareContainerAgentAdaptor();

    SoftwareContainerAgentAdaptor(SoftwareContainerAgent &agent);

    uint32_t LaunchCommand(const uint32_t &containerID, const uint32_t &userID, const std::string &commandLine,
                const std::string &workingDirectory,
                const std::string &outputFile,
                const std::map<std::string,
                    std::string> &env);

    void ShutDownContainerWithTimeout(const uint32_t &containerID, const uint32_t &timeout);

    void ShutDownContainer(const uint32_t &containerID) override;

    std::string BindMountFolderInContainer(const uint32_t &containerID, const std::string &pathInHost,
                const std::string &subPathInContainer, const bool &readOnly) override;

    void SetGatewayConfigs(const uint32_t &containerID, const std::map<std::string, std::string> &configs) override;

    bool SetCapabilities(const uint32_t &containerID, const std::vector<std::string> &capabilities) override;

    uint32_t CreateContainer(const std::string &name, const std::string &config) override;

    void SetContainerName(const uint32_t &containerID, const std::string &name) override;

    void Ping() override;

    void WriteToStdIn(const uint32_t &containerID, const std::vector<uint8_t> &bytes) override;

    SoftwareContainerAgent &m_agent;

};
}


// Utility class for DBus Adaptors
class DBusCppAdaptor: public SoftwareContainerAgentAdaptor, public DBus::IntrospectableAdaptor, public DBus::ObjectAdaptor {
public:
    DBusCppAdaptor(DBus::Connection& connection, const std::string& objectPath, SoftwareContainerAgent &agent) :
        SoftwareContainerAgentAdaptor(agent), DBus::ObjectAdaptor(connection, objectPath)
    {
    }
};

#endif // SOFTWARECONTAINERAGENTADAPTOR_H
