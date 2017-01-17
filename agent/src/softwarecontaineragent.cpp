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
#include "config/configerror.h"
#include "config/configdefinition.h"
#include "softwarecontainererror.h"

namespace softwarecontainer {


SoftwareContainerAgent::SoftwareContainerAgent(
        Glib::RefPtr<Glib::MainContext> mainLoopContext,
        const Config &config):
    m_mainLoopContext(mainLoopContext)
{
    m_containerIdPool.push_back(0);

    // Get all configs for this objects members
    try {
        m_preloadCount = config.getIntValue(ConfigDefinition::SC_GROUP,
                                            ConfigDefinition::SC_PRELOAD_COUNT_KEY);

    } catch (ConfigError &error) {
        throw ReturnCode::FAILURE;
    }

    // Get all configs for the workspace
    int shutdownTimeout;
    std::string containerRootDir;
    std::string lxcConfigPath;
    try {
        shutdownTimeout = config.getIntValue(ConfigDefinition::SC_GROUP,
                                             ConfigDefinition::SC_SHUTDOWN_TIMEOUT_KEY);
        containerRootDir = config.getStringValue(ConfigDefinition::SC_GROUP,
                                                 ConfigDefinition::SC_SHARED_MOUNTS_DIR_KEY);
        lxcConfigPath = config.getStringValue(ConfigDefinition::SC_GROUP,
                                              ConfigDefinition::SC_LXC_CONFIG_PATH_KEY);
    } catch (ConfigError &error) {
        throw ReturnCode::FAILURE;
    }

    // Create and set all values on workspace
    try {
        m_softwarecontainerWorkspace = std::make_shared<Workspace>();
        m_softwarecontainerWorkspace->m_enableWriteBuffer = false;
        m_softwarecontainerWorkspace->m_containerRootDir = containerRootDir;
        m_softwarecontainerWorkspace->m_containerConfigPath = lxcConfigPath;
        m_softwarecontainerWorkspace->m_containerShutdownTimeout = shutdownTimeout;
    } catch (ReturnCode err) {
        log_error() << "Failed to set up workspace";
        throw ReturnCode::FAILURE;
    }

    if (!triggerPreload()) {
        log_error() << "Failed to preload";
        throw ReturnCode::FAILURE;
    }

    // Get all configs for the config stores
    std::string serviceManifestDir;
    std::string defaultServiceManifestDir;
    try {
        serviceManifestDir = config.getStringValue(ConfigDefinition::SC_GROUP,
                                                   ConfigDefinition::SC_SERVICE_MANIFEST_DIR_KEY);
        defaultServiceManifestDir = config.getStringValue(ConfigDefinition::SC_GROUP,
                                                          ConfigDefinition::SC_DEFAULT_SERVICE_MANIFEST_DIR_KEY);
    } catch (ConfigError &error) {
        throw ReturnCode::FAILURE;
    }

    try {
        m_filteredConfigStore = std::make_shared<FilteredConfigStore>(serviceManifestDir);
        m_defaultConfigStore = std::make_shared<DefaultConfigStore>(defaultServiceManifestDir);
    } catch (ReturnCode err) {
        log_error() << "Failed to initialize ConfigStore";
        throw ReturnCode::FAILURE;
    }
}

SoftwareContainerAgent::~SoftwareContainerAgent()
{
}

bool SoftwareContainerAgent::triggerPreload()
{
    while (m_preloadedContainers.size() < m_preloadCount) {
        auto availableID = findSuitableId();
        SoftwareContainerPtr container = std::move(makeSoftwareContainer(availableID));

        if (isError(container->preload())) {
            log_error() << "Preloading failed";
            return false;
        }
        auto pair = std::pair<ContainerID, SoftwareContainerPtr>(availableID, std::move(container));
        m_preloadedContainers.push(std::move(pair));
    }

    return true;
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

void SoftwareContainerAgent::readConfigElement(const json_t *element)
{
    if (!json_is_object(element)) {
        std::string errorMessage("Configure entry is not an object");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    bool wo = false;
    if(!JSONParser::read(element, "enableWriteBuffer", wo)) {
        std::string errorMessage("Could not parse config due to: 'enableWriteBuffer' not found.");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    m_softwarecontainerWorkspace->m_enableWriteBuffer = wo;
}

void SoftwareContainerAgent::parseConfig(const std::string &config)
{
    if (config.size() == 0) {
        std::string errorMessage("Empty JSON config strings are not supported.");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    if (!root) {
        std::string errorMessage("Could not parse config: "
                                + std::string(error.text)
                                + config);
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    if (!json_is_array(root)) {
        std::string errorMessage("Root JSON element is not an array");
        log_error() << errorMessage;
        json_decref(root);
        throw SoftwareContainerError(errorMessage);
    }

    size_t index;
    json_t *element;

    try {
        json_array_foreach(root, index, element) {
            readConfigElement(element);
        }
    } catch (SoftwareContainerError &err) {
        json_decref(root);
        throw;
    }

    json_decref(root);
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

SoftwareContainerAgent::SoftwareContainerPtr SoftwareContainerAgent::makeSoftwareContainer(const ContainerID containerID)
{
    log_debug() << "Created container with ID :" << containerID;
    auto container = SoftwareContainerPtr(new SoftwareContainer(m_softwarecontainerWorkspace, containerID));
    return container;
}

ContainerID SoftwareContainerAgent::createContainer(const std::string &config)
{
    profilepoint("createContainerStart");
    profilefunction("createContainerFunction");

    try {
        parseConfig(config);
    } catch (SoftwareContainerError &err) {
        std::string errorMessage("The provided config could not be successfully parsed:" + std::string(err.what()));
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    std::pair<ContainerID, SoftwareContainerPtr> pair = std::move(getContainerPair());
    pair.second->setMainLoopContext(m_mainLoopContext);

    ContainerID containerID = pair.first;

    if (isError(pair.second->init())) {
        std::string errorMessage("Could not init the container" + std::to_string(containerID));
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    // TODO: Would be nice if this could be done async.
    if (!triggerPreload()) {
        log_warning() << "Failed to preload new container";
    }

    m_containers.insert(std::move(pair));

    return containerID;
}

std::pair<ContainerID, SoftwareContainerAgent::SoftwareContainerPtr> SoftwareContainerAgent::getContainerPair()
{
    if (!m_preloadedContainers.empty()) {
        auto pair = std::move(m_preloadedContainers.front());
        m_preloadedContainers.pop();

        return pair;
    }

    // Nothing is preloaded
    ContainerID availableID = findSuitableId();
    auto container = std::move(makeSoftwareContainer(availableID));
    return std::make_pair(availableID, std::move(container));
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
     * We want to always apply any default capabilities we have. If the container is in READY state,
     * that means that its gateways have been configured. The only way to configure the gateways
     * from the agent is through setCapabilities - which sets the default caps also.
     *
     * If it has not been set, then we use a call without arguments to setCapabilities to set it up,
     * since we then should get only the default ones.
     */
    if (container->getContainerState() != ContainerState::READY) {
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
    job->setOutputFile(outputFile);
    job->setEnvironmentVariables(env);
    job->setWorkingDirectory(workingDirectory);

    // Start it
    if (isError(job->start())) {
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

    int timeout = m_softwarecontainerWorkspace->m_containerShutdownTimeout;
    if (isError(container->shutdown(timeout))) {
        std::string errorMessage("Could not shut down the container");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

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

    ReturnCode result = container->bindMount(pathInHost,
                                             pathInContainer,
                                             readOnly);
    if (isError(result)) {
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

    ReturnCode result = container->startGateways(configs);
    return isSuccess(result);
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


std::shared_ptr<Workspace> SoftwareContainerAgent::getWorkspace()
{
    return m_softwarecontainerWorkspace;
}

} // namespace softwarecontainer
