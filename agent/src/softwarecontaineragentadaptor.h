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

#include "softwarecontaineragent.h"

namespace softwarecontainer {

class SoftwareContainerAgentAdaptor :
    public com::pelagicore::SoftwareContainerAgent_adaptor
{
    LOG_DECLARE_CLASS_CONTEXT("SCAA", "SoftwareContainerAgentAdaptor");

public:
    virtual ~SoftwareContainerAgentAdaptor();

    SoftwareContainerAgentAdaptor(SoftwareContainerAgent &agent);

    std::vector<int32_t> List() override;
    std::vector<std::string> ListCapabilities() override;

    void Execute(const int32_t &containerID,
                 const uint32_t &userID,
                 const std::string &commandLine,
                 const std::string &workingDirectory,
                 const std::string &outputFile,
                 const std::map<std::string, std::string> &env,
                 int32_t &pid,
                 bool &success);

    bool Destroy(const int32_t &containerID) override;

    bool BindMount(const int32_t &containerID,
                   const std::string &pathInHost,
                   const std::string &PathInContainer,
                   const bool &readOnly) override;

    bool Suspend(const int32_t &containerID) override;

    bool Resume(const int32_t &containerID) override;

    bool SetGatewayConfigs(const int32_t &containerID,
                           const std::map<std::string, std::string> &configs) override;

    bool SetCapabilities(const int32_t &containerID,
                         const std::vector<std::string> &capabilities) override;

    void Create(const std::string &config, int32_t &containerID, bool &success) override;

    SoftwareContainerAgent &m_agent;

};
}


// Utility class for DBus Adaptors
class DBusCppAdaptor : public SoftwareContainerAgentAdaptor,
                       public DBus::IntrospectableAdaptor,
                       public DBus::ObjectAdaptor {
public:
    DBusCppAdaptor(DBus::Connection &connection,
                   const std::string &objectPath,
                   SoftwareContainerAgent &agent) :
        SoftwareContainerAgentAdaptor(agent),
        DBus::ObjectAdaptor(connection, objectPath)
    {
    }
};

