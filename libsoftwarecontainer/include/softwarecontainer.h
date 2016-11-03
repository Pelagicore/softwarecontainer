
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
#include "workspace.h"
#include "gateway.h"
#include "container.h"

namespace softwarecontainer {

class SoftwareContainer :
    private FileToolkitWithUndo
{
public:
    LOG_DECLARE_CLASS_CONTEXT("PCL", "SoftwareContainer library");

    SoftwareContainer(std::shared_ptr<Workspace> workspace, const std::string &id);

    ~SoftwareContainer();

    /**
     * @brief Set the main loop
     */
    void setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext);

    /**
     * @brief Shutdown the container
     */
    ReturnCode shutdown();
    ReturnCode shutdown(unsigned int timeout);

    /*
     * @brief suspend the container
     *
     * This suspends any execution inside the container until resume is called.
     *
     * @return ReturnCode::FAILURE if the container was already suspended
     * @return ReturnCode::SUCCESS if the container was successfully suspended
     */
    ReturnCode suspend();

    /*
     * @brief resume a suspended container
     *
     * This resumes execution of a container that was suspended.
     *
     * @return ReturnCode::FAILURE if the container was not suspended
     * @return ReturnCode::SUCCESS if the container was successfully resumed
     */
    ReturnCode resume();

    bool isInitialized() const;

    void addGateway(Gateway *gateway);

    /**
     * @brief Preload the container.
     * That method can be called before setting the main loop context
     */
    ReturnCode preload();
    ReturnCode init();

    std::shared_ptr<ContainerAbstractInterface> getContainer();
    std::string getContainerDir();
    std::string getGatewayDir();

    void setContainerName(const std::string &name);

    ObservableProperty<ContainerState> &getContainerState();

    /**
     * @brief Continues the 'launch' phase by allowing gateway configurations to
     *  be set.
     *
     * Platform Access Manager calls this method after SoftwareContainer has
     * registered as a client, and passes all gateway configurations as
     * argument. SoftwareContainer sets the gateway configurations and activates
     * all gateways as a result of this call. The contained application
     * is then started.
     *
     * @param configs A map of gateway IDs and their respective configurations
     */
    void updateGatewayConfiguration(const GatewayConfiguration &configs);

    /**
     * @brief Set the gateway configuration and activate them
     */
    void setGatewayConfigs(const GatewayConfiguration &configs);

private:
    ReturnCode shutdownGateways();

    std::shared_ptr<Workspace> m_workspace;

    std::string m_containerName;
    ObservableWritableProperty<ContainerState> m_containerState;

    std::shared_ptr<ContainerAbstractInterface> m_container;
    pid_t m_pcPid = INVALID_PID;
    std::vector<std::unique_ptr<Gateway>> m_gateways;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    SignalConnectionsHandler m_connections;

    bool m_initialized = false;
};
}
