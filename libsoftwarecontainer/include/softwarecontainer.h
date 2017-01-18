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

#include "observableproperty.h"
#include "workspace.h"
#include "gatewayconfig.h"
#include "signalconnectionshandler.h"

#include <glibmm.h>

#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "functionjob.h"
#include "commandjob.h"


namespace softwarecontainer {
class Gateway;
class ContainerAbstractInterface;

class SoftwareContainer :
    private FileToolkitWithUndo
{
public:
    LOG_DECLARE_CLASS_CONTEXT("PCL", "SoftwareContainer library");

    SoftwareContainer(std::shared_ptr<Workspace> workspace,
                      const ContainerID id,
                      std::string bridgeIp = "10.0.3.1",
                      int netmaskBits = 16);

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

    void addGateway(Gateway *gateway);

    bool isInitialized() const;

    ReturnCode start();

    ReturnCode init();

    std::shared_ptr<ContainerAbstractInterface> getContainer();

    std::shared_ptr<FunctionJob> createFunctionJob(const std::function<int()> fun);
    std::shared_ptr<CommandJob> createCommandJob(const std::string &command);

    ReturnCode bindMount(const std::string &pathOnHost, const std::string &pathInContainer, bool readonly = true);

    std::string getContainerDir();
    std::string getGatewayDir();

    ObservableProperty<ContainerState> &getContainerState();

    /**
     * @brief Starts the Gateways by setting the gateway configurations
     * and activating the configured gateway
     *
     * @return ReturnCode::FAILURE if configuration or activation failed
     */
    ReturnCode startGateways(const GatewayConfiguration &configs);

private:
    ReturnCode configureGateways(const GatewayConfiguration &gwConfig);
    ReturnCode activateGateways();
    ReturnCode shutdownGateways();

    std::shared_ptr<Workspace> m_workspace;
    ContainerID m_containerID;
    std::string m_bridgeIp;
    int m_netmaskBits;

    ObservableWritableProperty<ContainerState> m_containerState;

    std::shared_ptr<ContainerAbstractInterface> m_container;
    pid_t m_pcPid = INVALID_PID;

    std::vector<std::unique_ptr<Gateway>> m_gateways;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    SignalConnectionsHandler m_connections;

    bool m_initialized = false;
};

} // namespace softwarecontainer
