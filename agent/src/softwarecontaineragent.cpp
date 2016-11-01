#include "softwarecontaineragent.h"
#include <cstdint>

SoftwareContainerAgent::SoftwareContainerAgent(
        Glib::RefPtr<Glib::MainContext> mainLoopContext
        , int preloadCount
        , bool shutdownContainers
        , int shutdownTimeout)
: m_mainLoopContext(mainLoopContext)
, m_preloadCount(preloadCount)
, m_shutdownContainers(shutdownContainers)
{
    m_containerIdPool.push_back(0);
    m_softwarecontainerWorkspace = std::make_shared<Workspace>();
    m_softwarecontainerWorkspace->m_containerShutdownTimeout = shutdownTimeout;

    if (isError(m_softwarecontainerWorkspace->checkWorkspace())) {
        log_error() << "Failed to set up workspace";
        throw ReturnCode::FAILURE;
    }

    if (!triggerPreload()) {
        log_error() << "Failed to preload";
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

inline bool SoftwareContainerAgent::isIdValid (ContainerID containerID)
{
    return ((containerID < UINT32_MAX)
            && (1 == m_containers.count(containerID)));
}

void SoftwareContainerAgent::deleteContainer(ContainerID containerID)
{
    if (isIdValid(containerID)) {
        m_containers.erase(containerID);
        m_containerIdPool.push_back(containerID);
    } else {
        log_error() << "Invalid container ID " << containerID;
    }
}

bool SoftwareContainerAgent::checkContainer(ContainerID containerID, SoftwareContainer *&container)
{
    if (isIdValid(containerID)) {
        container = m_containers[containerID].get();
        return true;
    } else {
        log_error() << "Invalid container ID " << containerID;
    }

    return false;
}

ReturnCode SoftwareContainerAgent::readConfigElement(const json_t *element)
{
    bool wo = false;
    if(!read(element, "enableWriteBuffer", wo)) {
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

    if (json_is_array(root)) {
        size_t index;
        json_t *element;
        json_array_foreach(root, index, element) {
            if (json_is_object(element)) {
                if (isError(readConfigElement(element))) {
                    log_error() << "Could not read config element";
                    return false;
                }
            } else {
                log_error() << "json configuration is not an object";
                return false;
            }
        }
    } else {
        log_error() << "Root JSON element is not an array";
        return false;
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

SoftwareContainerAgent::SoftwareContainerPtr SoftwareContainerAgent::makeSoftwareContainer(const ContainerID &containerID)
{
    log_debug() << "Created container with ID :" << containerID;
    auto container = SoftwareContainerPtr(new SoftwareContainer(m_softwarecontainerWorkspace, "SC-" + std::to_string(containerID)));
    return container;
}

ContainerID SoftwareContainerAgent::createContainer(const std::string &config)
{
    profilepoint("createContainerStart");
    profilefunction("createContainerFunction");

    if (!parseConfig(config)) {
        log_error() << "The provided config could not be successfully parsed.";
        return -1;
    }

    std::pair<ContainerID, SoftwareContainerPtr> pair = std::move(getContainerPair());
    pair.second->setMainLoopContext(m_mainLoopContext);

    if (isError(pair.second->init())) {
        log_error() << "Could not init the container.";
        return -1;
    }

    // TODO: Would be nice if this could be done async.
    if (!triggerPreload()) {
        log_warning() << "Failed to preload a new container.";
    }

    m_containers.insert(std::move(pair));

    // Return the ContainerID for the container
    return pair.first;
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

void SoftwareContainerAgent::writeToStdIn(pid_t pid, const std::vector<uint8_t> &bytes)
{
    CommandJob *job = nullptr;
    if (checkJob(pid, job)) {
        log_debug() << "writing bytes to process with PID:" << job->pid() << " : " << bytes;
        write(job->stdin(), bytes.data(), bytes.size());
    }
}

pid_t SoftwareContainerAgent::launchCommand(ContainerID containerID, uid_t userID, const std::string &cmdLine, const std::string &workingDirectory, const std::string &outputFile, const EnvironmentVariables &env, std::function<void (pid_t, int)> listener)
{
    profilefunction("launchCommandFunction");
    SoftwareContainer *container;
    if (checkContainer(containerID, container)) {
        auto job = new CommandJob(*container, cmdLine);
        // Capturing this leaves an open pipe for every container, even
        // after it has terminated.
        // job->captureStdin();
        job->setOutputFile(outputFile);
        job->setUserID(userID);
        job->setEnvironmentVariables(env);
        job->setWorkingDirectory(workingDirectory);
        job->start();
        addProcessListener(m_connections, job->pid(), [listener](pid_t pid, int exitCode) {
            listener(pid, exitCode);
        }, m_mainLoopContext);

        m_jobs.push_back(job);

        profilepoint("launchCommandEnd");

        return job->pid();
    }
    return INVALID_PID;
}

void SoftwareContainerAgent::setContainerName(ContainerID containerID, const std::string &name)
{
    SoftwareContainer *container = nullptr;
    if (checkContainer(containerID, container)) {
        container->setContainerName(name);
    }
}

void SoftwareContainerAgent::shutdownContainer(ContainerID containerID)
{
    shutdownContainer(containerID, m_softwarecontainerWorkspace->m_containerShutdownTimeout);
}

void SoftwareContainerAgent::shutdownContainer(ContainerID containerID, unsigned int timeout)
{
    profilefunction("shutdownContainerFunction");
    if (m_shutdownContainers) {
        SoftwareContainer *container = nullptr;
        if (checkContainer(containerID, container)) {
            container->shutdown(timeout);
            deleteContainer(containerID);
        }
    } else {
        log_info() << "Not shutting down container";
    }
}

std::string SoftwareContainerAgent::bindMountFolderInContainer(const uint32_t containerID, const std::string &pathInHost, const std::string &subPathInContainer, bool readOnly)
{
    profilefunction("bindMountFolderInContainerFunction");
    SoftwareContainer *container = nullptr;
    if (checkContainer(containerID, container)) {
        std::string path;
        ReturnCode result = container->getContainer()->bindMountFolderInContainer(pathInHost, subPathInContainer, path, readOnly);
        if (isError(result)) {
            log_error() << "Unable to bind mount folder " << pathInHost << " to " << subPathInContainer;
            return "";
        }

        return path;
    }
    return "";
}

void SoftwareContainerAgent::setGatewayConfigs(const uint32_t &containerID, const std::map<std::string, std::string> &configs)
{
    profilefunction("setGatewayConfigsFunction");
    SoftwareContainer *container = nullptr;
    if (checkContainer(containerID, container)) {
        container->updateGatewayConfiguration(configs);
    }
}

bool SoftwareContainerAgent::setCapabilities(const uint32_t &containerID, const std::vector<std::string> &capabilities)
{
    return true;
}


std::shared_ptr<Workspace> SoftwareContainerAgent::getWorkspace()
{
    return m_softwarecontainerWorkspace;
}
