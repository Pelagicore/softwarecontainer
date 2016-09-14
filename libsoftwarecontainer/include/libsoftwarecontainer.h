
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

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <glibmm.h>

#include "gateway.h"

namespace softwarecontainer {

class SoftwareContainerWorkspace :
    private FileToolkitWithUndo
{
    LOG_DECLARE_CLASS_CONTEXT("PCLW", "SoftwareContainer library workspace");

public:
    SoftwareContainerWorkspace(
            bool enableWriteBuffer = false,
            const std::string &containerRootFolder = SOFTWARECONTAINER_DEFAULT_WORKSPACE,
            const std::string &configFilePath = SOFTWARECONTAINER_DEFAULT_CONFIG,
            unsigned int containerShutdownTimeout = 2)
        : m_enableWriteBuffer(enableWriteBuffer)
        , m_containerRoot(containerRootFolder)
        , m_containerConfig(configFilePath)
        , m_containerShutdownTimeout(containerShutdownTimeout)
    {
        // Make sure path ends in '/' since it might not always be checked
        if (m_containerRoot.back() != '/') {
            m_containerRoot += "/";
        }

        if (isError(checkWorkspace())) {
            log_error() << "Failed when checking workspace";
            assert(false);
        }
    }

    ~SoftwareContainerWorkspace()
    {
    }

    /**
     * Check if the workspace is present and create it if needed
     */
    ReturnCode checkWorkspace();

    bool m_enableWriteBuffer;
    std::string m_containerRoot;
    std::string m_containerConfig;
    unsigned int m_containerShutdownTimeout;
};

SoftwareContainerWorkspace &getDefaultWorkspace(bool enableWriteBuffer);

class SoftwareContainerLib :
    private FileToolkitWithUndo
{
public:
    LOG_DECLARE_CLASS_CONTEXT("PCL", "SoftwareContainer library");

    SoftwareContainerLib(SoftwareContainerWorkspace &workspace = getDefaultWorkspace(false));

    ~SoftwareContainerLib();

    /**
     * Set the main loop
     */
    void setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext);

    /* 
     * Shutdown the container
     */ 
    ReturnCode shutdown();
    ReturnCode shutdown(unsigned int timeout);

    bool isInitialized() const;

    void addGateway(Gateway *gateway);

    /**
     * Preload the container.
     * That method can be called before setting the main loop context
     */
    ReturnCode preload();
    ReturnCode init();

    std::shared_ptr<ContainerAbstractInterface> getContainer();
    const std::string &getContainerID();
    std::string getContainerDir();
    std::string getGatewayDir();

    void setContainerIDPrefix(const std::string &prefix);
    void setContainerName(const std::string &name);

    void validateContainerID();

    ObservableProperty<ContainerState> &getContainerState();

    pid_t launchCommand(const std::string &commandLine);

    /*! Continues the 'launch' phase by allowing gateway configurations to
     *  be set.
     *
     * Platform Access Manager calls this method after SoftwareContainer has
     * registered as a client, and passes all gateway configurations as
     * argument. SoftwareContainer sets the gateway configurations and activates
     * all gateways as a result of this call. The contained application
     * is then started.
     *
     * \param configs A map of gateway IDs and their respective configurations
     */
    void updateGatewayConfiguration(const GatewayConfiguration &configs);

    /**
     * Set the gateway configuration and activate them
     */
    void setGatewayConfigs(const GatewayConfiguration &configs);

private:
    ReturnCode shutdownGateways();

    SoftwareContainerWorkspace &m_workspace;

    std::string m_containerID;
    std::string m_containerName;
    ObservableWritableProperty<ContainerState> m_containerState;

    std::shared_ptr<ContainerAbstractInterface> m_container;
    pid_t m_pcPid = INVALID_PID;
    std::vector<std::unique_ptr<Gateway> > m_gateways;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    SignalConnectionsHandler m_connections;

    bool m_initialized = false;
};
}
