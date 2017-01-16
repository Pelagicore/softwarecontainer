/*
 * Copyright (C) 2016-2017 Pelagicore AB
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

#include "softwarecontaineragentadaptor.h"

namespace softwarecontainer {

SoftwareContainerAgentAdaptor::~SoftwareContainerAgentAdaptor()
{
}

SoftwareContainerAgentAdaptor::SoftwareContainerAgentAdaptor(::softwarecontainer::SoftwareContainerAgent &agent, bool useSessionBus) :
    com::pelagicore::SoftwareContainerAgent(), m_agent(agent)
{
    std::string agentBusName = "com.pelagicore.SoftwareContainerAgent";
    Gio::DBus::BusType busType = useSessionBus ? Gio::DBus::BUS_TYPE_SESSION
                                               : Gio::DBus::BUS_TYPE_SYSTEM;
    connect(busType, agentBusName);
}

void SoftwareContainerAgentAdaptor::List(SoftwareContainerAgentMessageHelper msg)
{
    std::vector<int> list = m_agent.listContainers();
    msg.returnValue(list);
}

void SoftwareContainerAgentAdaptor::ListCapabilities(SoftwareContainerAgentMessageHelper msg)
{
    std::vector<std::string> stdStrVec = m_agent.listCapabilities();
    std::vector<Glib::ustring> glibStrVec = SoftwareContainerAgentCommon::stdStringVecToGlibStringVec(stdStrVec);
    msg.returnValue(glibStrVec);
}

void SoftwareContainerAgentAdaptor::Execute(
    const gint32 containerID,
    const std::string commandLine,
    const std::string workingDirectory,
    const std::string outputFile,
    const std::map<std::string, std::string> env,
    SoftwareContainerAgentMessageHelper msg)
{
    pid_t pid = m_agent.execute(
        containerID,
        commandLine,
        workingDirectory,
        outputFile,
        env,
        [this, containerID](pid_t pid, int exitCode) {
            ProcessStateChanged_emitter(containerID, pid, false, exitCode);
            log_info() << "ProcessStateChanged " << pid << " code " << exitCode;
        }
    );
    msg.returnValue(pid);
}

void SoftwareContainerAgentAdaptor::Suspend(const gint32 containerID, SoftwareContainerAgentMessageHelper msg)
{
    m_agent.suspendContainer(containerID);
    msg.returnValue();
}

void SoftwareContainerAgentAdaptor::Resume(const gint32 containerID, SoftwareContainerAgentMessageHelper msg)
{
    m_agent.resumeContainer(containerID);
    msg.returnValue();
}

void SoftwareContainerAgentAdaptor::Destroy(const gint32 containerID, SoftwareContainerAgentMessageHelper msg)
{
    m_agent.shutdownContainer(containerID);
    msg.returnValue();
}

void SoftwareContainerAgentAdaptor::BindMount(
    const gint32 containerID,
    const std::string pathInHost,
    const std::string PathInContainer,
    const bool readOnly,
    SoftwareContainerAgentMessageHelper msg)
{
    m_agent.bindMount(containerID, pathInHost, PathInContainer, readOnly);
    msg.returnValue();
}

void SoftwareContainerAgentAdaptor::SetCapabilities(
    const gint32 containerID,
    const std::vector<std::string> capabilities,
    SoftwareContainerAgentMessageHelper msg)
{
    m_agent.setCapabilities(containerID, capabilities);
    msg.returnValue();
}

void SoftwareContainerAgentAdaptor::Create(const std::string config,
                                           SoftwareContainerAgentMessageHelper msg)
{
    gint32 containerID = m_agent.createContainer(config);
    msg.returnValue(containerID);
}

} // namespace softwarecontainer
