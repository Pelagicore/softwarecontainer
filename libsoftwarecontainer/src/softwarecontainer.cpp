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


#include "softwarecontainer.h"
#include "generators.h" /* used for gen_ct_name */

#ifdef ENABLE_PULSEGATEWAY
#include "gateway/pulsegateway.h"
#endif

#ifdef ENABLE_NETWORKGATEWAY
#include "gateway/networkgateway.h"
#endif

#ifdef ENABLE_DBUSGATEWAY
#include "gateway/dbusgateway.h"
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
#include "gateway/devicenodegateway.h"
#endif

#ifdef ENABLE_CGROUPSGATEWAY
#include "gateway/cgroupsgateway.h"
#endif

#include "gateway/envgateway.h"
#include "gateway/waylandgateway.h"
#include "gateway/filegateway.h"

namespace softwarecontainer {

SoftwareContainer::SoftwareContainer(std::shared_ptr<Workspace> workspace) :
    m_workspace(workspace),
    m_container(new Container(getContainerID()
                              , m_containerName
                              , m_workspace->m_containerConfigPath
                              , m_workspace->m_containerRootDir
                              , m_workspace->m_enableWriteBuffer
                              , m_workspace->m_containerShutdownTimeout))
{
    m_containerState = ContainerState::CREATED;
}

SoftwareContainer::~SoftwareContainer()
{
}

void SoftwareContainer::setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext)
{
    m_mainLoopContext = mainLoopContext;
}


void SoftwareContainer::setContainerIDPrefix(const std::string &name)
{
    m_containerID = name + Generator::gen_ct_name();
    log_debug() << "Assigned container ID " << m_containerID;
}

void SoftwareContainer::setContainerName(const std::string &name)
{
    m_containerName = name;
    log_debug() << m_container->toString();
}

void SoftwareContainer::validateContainerID()
{
    if (m_containerID.size() == 0) {
        setContainerIDPrefix("PLC-");
    }
}


ReturnCode SoftwareContainer::preload()
{
    log_debug() << "Initializing container";
    if (isError(m_container->initialize())) {
        log_error() << "Could not setup container for preloading";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Creating container";
    if (isError(m_container->create())) {
        return ReturnCode::FAILURE;
    }

    log_debug() << "Starting container";
    ReturnCode result = m_container->start(&m_pcPid);
    if (isError(result)) {
        log_error() << "Could not start the container during preload";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Started container with PID " << m_pcPid;
    m_containerState.setValueNotify(ContainerState::PRELOADED);
    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainer::init()
{
    validateContainerID();

    if (m_mainLoopContext->gobj() == nullptr) {
        log_error() << "Main loop context must be set first !";
        return ReturnCode::FAILURE;
    }

    if (getContainerState() != ContainerState::PRELOADED) {
        if (isError(preload())) {
            log_error() << "Failed to preload container";
            return ReturnCode::FAILURE;
        }
    }

#ifdef ENABLE_NETWORKGATEWAY
    addGateway(new NetworkGateway());
#endif

#ifdef ENABLE_PULSEGATEWAY
    addGateway(new PulseGateway());
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
    addGateway(new DeviceNodeGateway());
#endif

#ifdef ENABLE_DBUSGATEWAY
    addGateway(new DBusGateway( DBusGateway::SessionProxy, getGatewayDir(), getContainerID()));
    addGateway(new DBusGateway( DBusGateway::SystemProxy, getGatewayDir(), getContainerID()));
#endif

#ifdef ENABLE_CGROUPSGATEWAY
    addGateway(new CgroupsGateway());
#endif

    addGateway(new WaylandGateway());
    addGateway(new FileGateway());
    addGateway(new EnvironmentGateway());

    m_initialized = true;
    return ReturnCode::SUCCESS;
}

void SoftwareContainer::addGateway(Gateway *gateway)
{
    gateway->setContainer(m_container);
    m_gateways.push_back(std::unique_ptr<Gateway>(gateway));
}

pid_t SoftwareContainer::launchCommand(const std::string &commandLine)
{
    log_debug() << "launchCommand called with commandLine: " << commandLine;
    pid_t pid = INVALID_PID;
    ReturnCode result = m_container->attach(commandLine, &pid);
    if (isError(result)) {
        log_error() << "Attach returned invalid pid, launchCommand fails";
        return INVALID_PID;
    }

    // TODO: Why do we shutdown as soon as one process exits?
    addProcessListener(m_connections, pid, [&](pid_t pid, int returnCode) {
        shutdown();
    }, m_mainLoopContext);

    return pid;
}

void SoftwareContainer::updateGatewayConfiguration(const GatewayConfiguration &configs)
{
    log_debug() << "updateGatewayConfiguration called" << configs;
    setGatewayConfigs(configs);
}

void SoftwareContainer::setGatewayConfigs(const GatewayConfiguration &configs)
{
    // Go through the active gateways and check if there is a configuration for it
    // If there is, apply it.

    for (auto &gateway : m_gateways) {
        std::string gatewayId = gateway->id();

        if (configs.count(gatewayId) != 0) {
            std::string config = configs.at(gatewayId);
            gateway->setConfig(config);
        }
    }

    for (auto &gateway : m_gateways) {
        if (gateway->isConfigured()) {
            gateway->activate();
        }
    }

    m_containerState.setValueNotify(ContainerState::READY);
}

ReturnCode SoftwareContainer::shutdown()
{
    return shutdown(m_workspace->m_containerShutdownTimeout);
}

ReturnCode SoftwareContainer::shutdown(unsigned int timeout)
{
    log_debug() << "shutdown called"; // << logging::getStackTrace();
    if(isError(shutdownGateways())) {
        log_error() << "Could not shut down all gateways cleanly, check the log";
    }

    if(isError(m_container->destroy(timeout))) {
        log_error() << "Could not destroy the container during shutdown";
        return ReturnCode::FAILURE;
    }

    m_containerState.setValueNotify(ContainerState::TERMINATED);
    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainer::shutdownGateways()
{
    ReturnCode status = ReturnCode::SUCCESS;
    for (auto &gateway : m_gateways) {
        if (gateway->isActivated()) {
            if (!gateway->teardown()) {
                log_warning() << "Could not tear down gateway cleanly: " << gateway->id();
                status = ReturnCode::FAILURE;
            }
        }
    }

    m_gateways.clear();
    return status;
}

bool SoftwareContainer::isInitialized() const
{
    return m_initialized;
}

std::shared_ptr<ContainerAbstractInterface> SoftwareContainer::getContainer()
{
    std::shared_ptr<ContainerAbstractInterface> ptrCopy = m_container;
    return ptrCopy;
}

std::string SoftwareContainer::getContainerDir()
{
    return m_workspace->m_containerRootDir + "/" + getContainerID();
}

std::string SoftwareContainer::getGatewayDir()
{
    return getContainerDir() + "/gateways";
}

const std::string &SoftwareContainer::getContainerID()
{
    return m_containerID;
}

ObservableProperty<ContainerState> &SoftwareContainer::getContainerState()
{
    return m_containerState;
}

}
