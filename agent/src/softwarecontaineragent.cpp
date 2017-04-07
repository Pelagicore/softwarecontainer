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

#include <cstdint>
#include "softwarecontaineragent.h"

#include "softwarecontainererror.h"

#include "config/configerror.h"
#include "config/configdefinition.h"
#include "config/softwarecontainerconfig.h"
#include "capability/servicemanifestfileloader.h"

namespace softwarecontainer {

SoftwareContainerAgent::SoftwareContainerAgent(Glib::RefPtr<Glib::MainContext> mainLoopContext,
                                               std::shared_ptr<Config> config,
                                               std::shared_ptr<SoftwareContainerFactory> factory,
                                               std::shared_ptr<ContainerUtilityInterface> utility):
    m_mainLoopContext(mainLoopContext),
    m_config(config),
    m_factory(factory),
    m_containerUtility(utility)
{
    m_containerIdPool.push_back(0);

    // Get all configs for the config object
    int shutdownTimeout =
        m_config->getIntValue(ConfigDefinition::SC_GROUP,
                              ConfigDefinition::SC_SHUTDOWN_TIMEOUT_KEY);
    std::string sharedMountsDir =
        m_config->getStringValue(ConfigDefinition::SC_GROUP,
                                 ConfigDefinition::SC_SHARED_MOUNTS_DIR_KEY);
    std::string lxcConfigPath =
        m_config->getStringValue(ConfigDefinition::SC_GROUP,
                                 ConfigDefinition::SC_LXC_CONFIG_PATH_KEY);
#ifdef ENABLE_NETWORKGATEWAY
        // All of the below are network settings
    bool createBridge = m_config->getBoolValue(ConfigDefinition::SC_GROUP,
                                               ConfigDefinition::SC_CREATE_BRIDGE_KEY);
    std::string bridgeDevice = m_config->getStringValue(ConfigDefinition::SC_GROUP,
                                                        ConfigDefinition::SC_BRIDGE_DEVICE_KEY);
    std::string bridgeIp = m_config->getStringValue(ConfigDefinition::SC_GROUP,
                                                    ConfigDefinition::SC_BRIDGE_IP_KEY);
    std::string bridgeNetmask = m_config->getStringValue(ConfigDefinition::SC_GROUP,
                                                         ConfigDefinition::SC_BRIDGE_NETMASK_KEY);
    int bridgeNetmaskBits = m_config->getIntValue(ConfigDefinition::SC_GROUP,
                                                  ConfigDefinition::SC_BRIDGE_NETMASK_BITLENGTH_KEY);
    std::string bridgeNetAddr = m_config->getStringValue(ConfigDefinition::SC_GROUP,
                                                 ConfigDefinition::SC_BRIDGE_NETADDR_KEY);
#endif // ENABLE_NETWORKGATEWAY

    // Get all configs for the config stores
    std::string serviceManifestDir =
        m_config->getStringValue(ConfigDefinition::SC_GROUP,
                                 ConfigDefinition::SC_SERVICE_MANIFEST_DIR_KEY);
    std::string defaultServiceManifestDir =
        m_config->getStringValue(ConfigDefinition::SC_GROUP,
                                 ConfigDefinition::SC_DEFAULT_SERVICE_MANIFEST_DIR_KEY);

    std::unique_ptr<ServiceManifestLoader> defaultLoader(new ServiceManifestFileLoader(defaultServiceManifestDir));
    std::unique_ptr<ServiceManifestLoader> loader(new ServiceManifestFileLoader(serviceManifestDir));

    m_filteredConfigStore = std::make_shared<FilteredConfigStore>(std::move(loader));
    m_defaultConfigStore  = std::make_shared<DefaultConfigStore>(std::move(defaultLoader));

    m_containerUtility->removeOldContainers();
    m_containerUtility->checkWorkspace();

    m_containerConfig = SoftwareContainerConfig(
#ifdef ENABLE_NETWORKGATEWAY
                                                createBridge,
                                                bridgeDevice,
                                                bridgeIp,
                                                bridgeNetmask,
                                                bridgeNetmaskBits,
                                                bridgeNetAddr,
#endif // ENABLE_NETWORKGATEWAY
                                                lxcConfigPath,
                                                sharedMountsDir,
                                                shutdownTimeout);

}

SoftwareContainerAgent::~SoftwareContainerAgent()
{
}

void SoftwareContainerAgent::assertContainerExists(ContainerID containerID)
{
    if (containerID >= INT32_MAX) {
        std::string errorMessage("Invalid Container ID: "
                                + std::to_string(containerID)
                                + ". ID can not be greater than INT32");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    if (containerID < 0) {
        std::string errorMessage("Invalid Container ID: "
                                + std::to_string(containerID)
                                + ". ID can not be negative");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }
    if (1 > m_containers.count(containerID)) {
        std::string errorMessage("Invalid Container ID: "
                                + std::to_string(containerID)
                                + ". No container matching that ID exists.");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }
}

std::vector<ContainerID> SoftwareContainerAgent::listContainers()
{
    std::vector<ContainerID> containerIDs;
    for (auto &cont : m_containers) {
        containerIDs.push_back(cont.first);
    }

    return containerIDs;
}

void SoftwareContainerAgent::deleteContainer(ContainerID containerID)
{
    assertContainerExists(containerID);

    m_containers.erase(containerID);
    m_containerIdPool.push_back(containerID);
}

SoftwareContainerAgent::SoftwareContainerPtr SoftwareContainerAgent::getContainer(ContainerID containerID)
{
    assertContainerExists(containerID);
    return m_containers[containerID];
}


ContainerID SoftwareContainerAgent::findSuitableId()
{
    ContainerID availableID = m_containerIdPool.back();
    if (m_containerIdPool.size() > 1) {
        m_containerIdPool.pop_back();
    } else {
        m_containerIdPool[0]++;
    }

    return availableID;
}

ContainerID SoftwareContainerAgent::createContainer(const std::string &config)
{
    profilepoint("createContainerStart");
    profilefunction("createContainerFunction");

    // Set options for this container
    std::unique_ptr<DynamicContainerOptions> options = m_optionParser.parse(config);
    std::unique_ptr<SoftwareContainerConfig> containerConfig = options->toConfig(m_containerConfig);

    // Get an ID and create the container
    ContainerID containerID = findSuitableId();
    auto container = m_factory->createContainer(containerID, std::move(containerConfig));
    log_debug() << "Created container with ID :" << containerID;

    m_containers[containerID] = container;
    return containerID;
}

pid_t SoftwareContainerAgent::execute(ContainerID containerID,
                                     const std::string &cmdLine,
                                     const std::string &workingDirectory,
                                     const std::string &outputFile,
                                     const EnvironmentVariables &env,
                                     std::function<void (pid_t, int)> listener)
{
    profilefunction("executeFunction");
    SoftwareContainerPtr container = getContainer(containerID);

    /*
     * We want to always apply any default capabilities we have. The only way to configure
     * the gateways from the agent is through setCapabilities - which sets the default caps also.
     *
     * If it has not been set previously, then we use a call without arguments to setCapabilities
     * to set it up, since we then should get only the default ones.
     */
    if (!m_containers[containerID]->previouslyConfigured()) {
        log_info() << "Container not configured yet, configuring with default capabilities, if any";
        GatewayConfiguration gatewayConfigs = m_defaultConfigStore->configs();
        if (!updateGatewayConfigs(containerID, gatewayConfigs)) {
            std::string errorMessage("Could not set default capabilities on container " + std::to_string(containerID));
            log_error() << errorMessage;
            throw SoftwareContainerError(errorMessage);
        }

    }

    // Set up a CommandJob for this run in the container
    auto job = container->createCommandJob(cmdLine);
    if (nullptr == job) {
        std::string errorMessage("Could not create job instance for " + cmdLine);
        log_error() << errorMessage;
        throw SoftwareContainerAgentError(errorMessage);
    }

    job->setOutputFile(outputFile);
    job->setEnvironmentVariables(env);
    job->setWorkingDirectory(workingDirectory);

    // Start it
    if (!job->start()) {
        std::string errorMessage("Could not start job");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    // If things went well, do what we need when it exits
    addProcessListener(m_connections, job->pid(), [listener](pid_t pid, int exitCode) {
        listener(pid, exitCode);
    }, m_mainLoopContext);

    profilepoint("executeEnd");

    return job->pid();
}

void SoftwareContainerAgent::shutdownContainer(ContainerID containerID)
{
    profilefunction("shutdownContainerFunction");

    SoftwareContainerPtr container = getContainer(containerID);

    int timeout = m_containerConfig.containerShutdownTimeout();
    container->shutdown(timeout);

    try {
        deleteContainer(containerID);
    } catch (SoftwareContainerError &err) {
        std::string errorMessage("Could not delete the container" + std::string(err.what()));
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }
}

void SoftwareContainerAgent::suspendContainer(ContainerID containerID)
{
    SoftwareContainerPtr container = getContainer(containerID);
    container->suspend();
}

void SoftwareContainerAgent::resumeContainer(ContainerID containerID)
{
    SoftwareContainerPtr container = getContainer(containerID);
    container->resume();
}

void SoftwareContainerAgent::bindMount(const ContainerID containerID,
                                       const std::string &pathInHost,
                                       const std::string &pathInContainer,
                                       bool readOnly)
{
    profilefunction("bindMountFunction");
    SoftwareContainerPtr container = getContainer(containerID);

    bool result = container->bindMount(pathInHost,
                                       pathInContainer,
                                       readOnly);
    if (!result) {
        std::string errorMessage("Unable to bind mount " + pathInHost + " to " + pathInContainer);
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }
}

bool SoftwareContainerAgent::updateGatewayConfigs(const ContainerID &containerID,
                                                  const GatewayConfiguration &configs)
{
    profilefunction("updateGatewayConfigs");
    SoftwareContainerPtr container = getContainer(containerID);

    return container->startGateways(configs);
}

std::vector<std::string> SoftwareContainerAgent::listCapabilities()
{
    return m_filteredConfigStore->IDs();
}

void SoftwareContainerAgent::setCapabilities(const ContainerID &containerID,
                                             const std::vector<std::string> &capabilities)
{
    if (capabilities.empty()) {
        log_warning() << "Got empty list of capabilities";
        return;
    }

    // Log a list of all caps that were provided
    std::string caps = "";
    for (const std::string &capName : capabilities) {
        caps += " " + capName;
    }
    log_debug() << "Will attempt to set capabilities:" + caps;

    GatewayConfiguration gatewayConfigs = m_defaultConfigStore->configs();
    GatewayConfiguration filteredConfigs = m_filteredConfigStore->configsByID(capabilities);

    // If we get an empty config the user passed a non existent cap name
    if (filteredConfigs.empty()) {
        std::string errorMessage("One or more capabilities were not found");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    gatewayConfigs.append(filteredConfigs);

    // Update container gateway configuration
    if (!updateGatewayConfigs(containerID, gatewayConfigs)) {
        std::string errorMessage("Could not set gateway configuration for capability");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }
}

} // namespace softwarecontainer
