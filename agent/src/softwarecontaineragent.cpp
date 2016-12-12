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

#include "softwarecontaineragent.h"
#include <cstdint>


SoftwareContainerAgent::SoftwareContainerAgent(
        Glib::RefPtr<Glib::MainContext> mainLoopContext,
        const Config &config):
    m_mainLoopContext(mainLoopContext)
{
    m_containerIdPool.push_back(0);

    // Get all configs for this objects members
    try {
        m_preloadCount = config.getIntegerValue(Config::SC_GROUP,
                                                Config::PRELOAD_COUNT_KEY);
        //TODO: the inversion of the value here should be worked away,
        //      but doesn't seem to work anyway, see bug
        m_shutdownContainers = !config.getBooleanValue(Config::SC_GROUP,
                                                       Config::KEEP_CONTAINERS_ALIVE_KEY);
    } catch (softwarecontainer::ConfigError &error) {
        throw ReturnCode::FAILURE;
    }

    // Get all configs for the workspace
    int shutdownTimeout;
    std::string containerRootDir;
    std::string lxcConfigPath;
    try {
        shutdownTimeout = config.getIntegerValue(Config::SC_GROUP, Config::SHUTDOWN_TIMEOUT_KEY);
        containerRootDir = config.getStringValue(Config::SC_GROUP, Config::SHARED_MOUNTS_DIR_KEY);
        lxcConfigPath = config.getStringValue(Config::SC_GROUP, Config::LXC_CONFIG_PATH_KEY);
    } catch (softwarecontainer::ConfigError &error) {
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
        serviceManifestDir = config.getStringValue(Config::SC_GROUP,
                                                   Config::SERVICE_MANIFEST_DIR_KEY);
        defaultServiceManifestDir = config.getStringValue(Config::SC_GROUP,
                                                          Config::DEFAULT_SERVICE_MANIFEST_DIR_KEY);
    } catch (softwarecontainer::ConfigError &error) {
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

inline bool SoftwareContainerAgent::isIdValid(ContainerID containerID)
{
    return (containerID < INT32_MAX)
        && (containerID >= 0)
        && (1 == m_containers.count(containerID));
}

bool SoftwareContainerAgent::listContainers(std::vector<ContainerID> &containers)
{
    for (auto &cont : m_containers) {
        containers.push_back(cont.first);
    }

    return true;
}

bool SoftwareContainerAgent::deleteContainer(ContainerID containerID)
{
    if (!isIdValid(containerID)) {
        log_error() << "Invalid container ID " << containerID;
        return false;
    }

    m_containers.erase(containerID);
    m_containerIdPool.push_back(containerID);
    return true;
}

bool SoftwareContainerAgent::checkContainer(ContainerID containerID, SoftwareContainer *&container)
{
    if (!isIdValid(containerID)) {
        log_error() << "Invalid container ID " << containerID;
        return false;
    }

    container = m_containers[containerID].get();
    return true;
}

ReturnCode SoftwareContainerAgent::readConfigElement(const json_t *element)
{
    bool wo = false;
    if(!JSONParser::read(element, "enableWriteBuffer", wo)) {
        log_debug() << "enableWriteBuffer not found";
    }

    m_softwarecontainerWorkspace->m_enableWriteBuffer = wo;
    return ReturnCode::SUCCESS;
}

bool SoftwareContainerAgent::parseConfig(const std::string &config)
{
    if (config.size() == 0) {
        log_warning() << "No configuration interpreted";
        return false;
    }

    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    if (!root) {
        log_error() << "Could not parse config: " << error.text;
        log_error() << config;
        return false;
    }

    if (!json_is_array(root)) {
        log_error() << "Root JSON element is not an array";
        return false;
    }

    size_t index;
    json_t *element;
    json_array_foreach(root, index, element) {
        if (!json_is_object(element)) {
            log_error() << "json configuration is not an object";
            return false;
        }

        if (isError(readConfigElement(element))) {
            log_error() << "Could not read config element";
            return false;
        }
    }

    return true;
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

bool SoftwareContainerAgent::createContainer(const std::string &config, ContainerID &containerID)
{
    profilepoint("createContainerStart");
    profilefunction("createContainerFunction");

    if (!parseConfig(config)) {
        log_error() << "The provided config could not be successfully parsed";
        containerID = INVALID_CONTAINER_ID;
        return false;
    }

    std::pair<ContainerID, SoftwareContainerPtr> pair = std::move(getContainerPair());
    pair.second->setMainLoopContext(m_mainLoopContext);

    if (isError(pair.second->init())) {
        log_error() << "Could not init the container";
        containerID = INVALID_CONTAINER_ID;
        return false;
    }

    // TODO: Would be nice if this could be done async.
    if (!triggerPreload()) {
        log_warning() << "Failed to preload new container";
    }

    m_containers.insert(std::move(pair));

    containerID = pair.first;
    return true;
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

bool SoftwareContainerAgent::checkJob(pid_t pid, CommandJob *&result)
{
    for (auto &job : m_jobs) {
        if (job->pid() == pid) {
            result = job;
            return true;
        }
    }

    log_warning() << "Unknown PID: " << pid;
    return false;
}

bool SoftwareContainerAgent::execute(ContainerID containerID,
                                     uid_t userID,
                                     const std::string &cmdLine,
                                     const std::string &workingDirectory,
                                     const std::string &outputFile,
                                     const EnvironmentVariables &env,
                                     int32_t &pid,
                                     std::function<void (pid_t, int)> listener)
{
    profilefunction("executeFunction");
    SoftwareContainer *container = nullptr;
    if (!checkContainer(containerID, container)) {
        log_error() << "Invalid container ID " << containerID;
        pid = INVALID_PID;
        return false;
    }

    // Set up a CommandJob for this run in the container
    auto job = new CommandJob(*container, cmdLine);
    job->setOutputFile(outputFile);
    job->setUserID(userID);
    job->setEnvironmentVariables(env);
    job->setWorkingDirectory(workingDirectory);

    // Start it
    if (isError(job->start())) {
        log_error() << "Could not start job";
        pid = INVALID_PID;
        return false;
    }

    // If things went well, do what we need when it exits
    addProcessListener(m_connections, job->pid(), [listener](pid_t pid, int exitCode) {
        listener(pid, exitCode);
    }, m_mainLoopContext);

    // Save the job
    m_jobs.push_back(job);
    profilepoint("executeEnd");

    pid = job->pid();
    return true;
}

bool SoftwareContainerAgent::shutdownContainer(ContainerID containerID)
{
    profilefunction("shutdownContainerFunction");
    if (!m_shutdownContainers) {
        log_info() << "Not shutting down containers, so not shutting down " << containerID;
        return false;
    }

    SoftwareContainer *container = nullptr;
    if (!checkContainer(containerID, container)) {
        log_error() << "Invalid container ID " << containerID;
        return false;
    }

    int timeout = m_softwarecontainerWorkspace->m_containerShutdownTimeout;
    if (isError(container->shutdown(timeout))) {
        log_error() << "Could not shut down the container";
        return false;
    }

    if (!deleteContainer(containerID)) {
        log_error() << "Could not delete the container";
        return false;
    }

    return true;
}

bool SoftwareContainerAgent::suspendContainer(ContainerID containerID)
{
    SoftwareContainer *container = nullptr;
    if (!checkContainer(containerID, container)) {
        log_error() << "Can't suspend container " << containerID;
        return false;
    }

    return isSuccess(container->suspend());
}

bool SoftwareContainerAgent::resumeContainer(ContainerID containerID)
{
    SoftwareContainer *container = nullptr;
    if (!checkContainer(containerID, container)) {
        log_error() << "Can't resume container " << containerID;
        return false;
    }

    return isSuccess(container->resume());
}

bool SoftwareContainerAgent::bindMount(const ContainerID containerID,
                                       const std::string &pathInHost,
                                       const std::string &pathInContainer,
                                       bool readOnly)
{
    profilefunction("bindMountFunction");
    SoftwareContainer *container = nullptr;
    if (!checkContainer(containerID, container)) {
        log_error() << "Invalid container ID " << containerID;
        return false;
    }

    ReturnCode result = container->getContainer()->bindMountFolderInContainer(pathInHost, pathInContainer, readOnly);
    if (isError(result)) {
        log_error() << "Unable to bind mount folder " << pathInHost << " to " << pathInContainer;
        return false;
    }

    return true;
}

bool SoftwareContainerAgent::setGatewayConfigs(const ContainerID &containerID,
                                               const std::map<std::string, std::string> &configs)
{
    profilefunction("setGatewayConfigs");
    GatewayConfiguration parsedConfigs;

    for (auto const &it : configs) {
        json_error_t error;
        std::string gwID = it.first;
        std::string configStr = it.second;

        json_t *jConfigs = json_loads(configStr.c_str(), 0, &error);
        if (!jConfigs) {
            log_error() << "Could not parse config while setting gateway config: " << error.text;
            log_error() << configs;
            return false;
        }
        parsedConfigs.append(gwID, jConfigs);
    }
    return updateGatewayConfigs(containerID, parsedConfigs);
}

bool SoftwareContainerAgent::updateGatewayConfigs(const ContainerID &containerID,
                                                  const GatewayConfiguration &configs)
{
    profilefunction("updateGatewayConfigs");
    SoftwareContainer *container = nullptr;
    if (!checkContainer(containerID, container)) {
        log_error() << "Could not update gateway configuration. Container ("
                    << std::to_string(containerID) <<") does not exist";
        return false;
    }
    container->setGatewayConfigs(configs);
    return true;
}

std::vector<std::string> SoftwareContainerAgent::listCapabilities()
{
    return m_filteredConfigStore->IDs();
}

bool SoftwareContainerAgent::setCapabilities(const ContainerID &containerID,
                                             const std::vector<std::string> &capabilities)
{
    auto gatewayConfigs = m_defaultConfigStore->configs();
    auto filteredConfigs = m_filteredConfigStore->configsByID(capabilities);
    gatewayConfigs.append(filteredConfigs);

    // Update container gateway configuration
    if (!updateGatewayConfigs(containerID, gatewayConfigs)) {
        log_error() << "Could not set gateway configuration for capability";
        return false;
    }

    return true;
}


std::shared_ptr<Workspace> SoftwareContainerAgent::getWorkspace()
{
    return m_softwarecontainerWorkspace;
}
