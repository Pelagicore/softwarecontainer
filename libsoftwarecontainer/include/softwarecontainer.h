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
#include "gatewayconfig.h"
#include "signalconnectionshandler.h"
#include "config/softwarecontainerconfig.h"
#include "filetoolkitwithundo.h"

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


/**
 * @class SoftwareContainer
 *
 * @brief An abstraction of concrete container implementations
 */

class SoftwareContainer :
    private FileToolkitWithUndo
{
public:
    LOG_DECLARE_CLASS_CONTEXT("PCL", "SoftwareContainer library");

    /*
     * @brief creates a new SoftwareContainer instance.
     *
     * @param id the containerID to use.
     * @param config an object holding all needed settings for the container
     *
     * @throws SoftwareContainerError if unable to set up the needed directories
     *         or network settings for this container.
     */
    SoftwareContainer(const ContainerID id,
                      std::unique_ptr<const SoftwareContainerConfig> config);

    ~SoftwareContainer();

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
    ReturnCode start();

    ReturnCode configureGateways(const GatewayConfiguration &gwConfig);
    ReturnCode activateGateways();
    ReturnCode shutdownGateways();

    // Check if the workspace is sound, and set it up if it isn't
    void checkWorkspace();
#ifdef ENABLE_NETWORKGATEWAY
    void checkNetworkSettings();
#endif // ENABLE_NETWORKGATEWAY

    ContainerID m_containerID;
    std::shared_ptr<ContainerAbstractInterface> m_container;
    pid_t m_pcPid = INVALID_PID;
    ObservableWritableProperty<ContainerState> m_containerState;

    std::vector<std::unique_ptr<Gateway>> m_gateways;
    std::unique_ptr<const SoftwareContainerConfig> m_config;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    SignalConnectionsHandler m_connections;
    bool m_initialized = false;
};

} // namespace softwarecontainer
