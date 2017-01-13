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

#pragma once

#include "softwarecontaineragent.h"
#include "softwarecontaineragent_dbus_stub.h"

namespace softwarecontainer {

class SoftwareContainerAgentAdaptor :
    public ::com::pelagicore::SoftwareContainerAgent
{
    LOG_DECLARE_CLASS_CONTEXT("SCAA", "SoftwareContainerAgentAdaptor");

public:
    virtual ~SoftwareContainerAgentAdaptor();

    SoftwareContainerAgentAdaptor(::softwarecontainer::SoftwareContainerAgent &agent, bool useSessionBus);

    void List(SoftwareContainerAgentMessageHelper msg) override;
    void ListCapabilities(SoftwareContainerAgentMessageHelper msg) override;

    void Execute(const gint32 containerID,
                 const std::string commandLine,
                 const std::string workingDirectory,
                 const std::string outputFile,
                 const std::map<std::string, std::string> env,
                 SoftwareContainerAgentMessageHelper msg) override;

    void Destroy(const gint32 containerID, SoftwareContainerAgentMessageHelper msg) override;

    void BindMount(const gint32 containerID,
                   const std::string pathInHost,
                   const std::string PathInContainer,
                   const bool readOnly,
                   SoftwareContainerAgentMessageHelper msg) override;

    void Suspend(const gint32 containerID, SoftwareContainerAgentMessageHelper msg) override;

    void Resume(const gint32 containerID, SoftwareContainerAgentMessageHelper msg) override;

    void SetCapabilities(const gint32 containerID,
                         const std::vector<std::string> capabilities,
                         SoftwareContainerAgentMessageHelper msg) override;

    void Create(const std::string config, SoftwareContainerAgentMessageHelper msg) override;

    ::softwarecontainer::SoftwareContainerAgent &m_agent;

};

} // namespace softwarecontainer

