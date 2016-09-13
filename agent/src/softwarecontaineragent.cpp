#include "softwarecontaineragent.h"

SoftwareContainerAgent::SoftwareContainerAgent(
        Glib::RefPtr<Glib::MainContext> mainLoopContext
        , int preloadCount
        , bool shutdownContainers
        , int shutdownTimeout)
    : m_mainLoopContext(mainLoopContext)
    , m_preloadCount(preloadCount)
    , m_shutdownContainers(shutdownContainers)
{
    triggerPreload();
    m_softwarecontainerWorkspace.m_containerShutdownTimeout = shutdownTimeout;
}

SoftwareContainerAgent::~SoftwareContainerAgent()
{
}

void SoftwareContainerAgent::triggerPreload()
{
    //        log_debug() << "triggerPreload " << m_preloadCount - m_preloadedContainers.size();
    while (m_preloadedContainers.size() != m_preloadCount) {
        auto container = new SoftwareContainerLib(m_softwarecontainerWorkspace);
        container->setContainerIDPrefix("Preload-");
        container->preload();
        m_preloadedContainers.push_back(SoftwareContainerLibPtr(container));
    }
}

void SoftwareContainerAgent::deleteContainer(ContainerID containerID)
{
    bool valid = ((containerID < m_containers.size()) && (m_containers[containerID] != nullptr));
    if (valid) {
        m_containers[containerID] = nullptr;
    } else {
        log_error() << "Invalid container ID " << containerID;
    }
}

bool SoftwareContainerAgent::checkContainer(ContainerID containerID, SoftwareContainerLib *&container)
{
    bool valid = ((containerID < m_containers.size()) && (m_containers[containerID] != nullptr));
    if (valid) {
        container = m_containers[containerID].get();
    } else {
        log_error() << "Invalid container ID " << containerID;
    }

    return valid;
}

ReturnCode SoftwareContainerAgent::readConfigElement(const json_t *element)
{
    std::string wo;
    if(!read(element, "writeOften", wo)) {
        log_debug() << "writeOften not found";
        m_softwarecontainerWorkspace.m_writeOften = false;
    } else {
        if (wo == "1") {
            m_softwarecontainerWorkspace.m_writeOften = true;
        } else {
            m_softwarecontainerWorkspace.m_writeOften = false;
        }
    }
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
        for(size_t i = 0; i < json_array_size(root); i++) {
            json_t *element = json_array_get(root, i);
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

ContainerID SoftwareContainerAgent::createContainer(const std::string &prefix, const std::string &config)
{
    profilepoint("createContainerStart");
    profilefunction("createContainerFunction");

    parseConfig(config);

    SoftwareContainerLib *container;
    if (m_preloadedContainers.size() != 0) {
        container = m_preloadedContainers[0].release();
        m_preloadedContainers.erase(m_preloadedContainers.begin());
    } else {
        container = new SoftwareContainerLib(m_softwarecontainerWorkspace);
    }

    m_containers.push_back(SoftwareContainerLibPtr(container));
    auto id = m_containers.size() - 1;
    log_debug() << "Created container with ID :" << id;
    container->setContainerIDPrefix(prefix);
    container->setMainLoopContext(m_mainLoopContext);
    container->init();

    triggerPreload();

    return id;
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
    SoftwareContainerLib *container;
    if (checkContainer(containerID, container)) {
        auto job = new CommandJob(*container, cmdLine);
        // Capturing this leaves an open pipe for every container, even
        // after it has terminated.
        // job->captureStdin();
        job->setOutputFile(outputFile);
        job->setUserID(userID);
        job->setEnvironnmentVariables(env);
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
    SoftwareContainerLib *container = nullptr;
    if (checkContainer(containerID, container)) {
        container->setContainerName(name);
    }
}

void SoftwareContainerAgent::shutdownContainer(ContainerID containerID)
{
    shutdownContainer(containerID, m_softwarecontainerWorkspace.m_containerShutdownTimeout);
}

void SoftwareContainerAgent::shutdownContainer(ContainerID containerID, unsigned int timeout)
{
    profilefunction("shutdownContainerFunction");
    if (m_shutdownContainers) {
        SoftwareContainerLib *container = nullptr;
        if (checkContainer(containerID, container)) {
            container->shutdown();
            deleteContainer(containerID);
        }
    } else {
        log_info() << "Not shutting down container";
    }
}

std::string SoftwareContainerAgent::bindMountFolderInContainer(const uint32_t containerID, const std::string &pathInHost, const std::string &subPathInContainer, bool readOnly)
{
    profilefunction("bindMountFolderInContainerFunction");
    SoftwareContainerLib *container = nullptr;
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
    SoftwareContainerLib *container = nullptr;
    if (checkContainer(containerID, container)) {
        container->updateGatewayConfiguration(configs);
    }
}

SoftwareContainerWorkspace SoftwareContainerAgent::getSoftwareContainerWorkspace()
{
    return m_softwarecontainerWorkspace;
}
