/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

/**
 * That file contains an executable which register a DBUS service which lets client applications manipulate LXC containers.
 * This executable needs to run by the root user since that is required by LXC.
 */

#include <glibmm.h>
#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

#include "CommandLineParser.h"
#include "pelagicore-log.h"

#include "ivi-main-loop/ivi-main-loop-unix-signal.h"

#include "pelagicore-DBusCpp.h"
#include "PelagicontainAgent_dbuscpp_adaptor.h"

#include "libpelagicontain.h"

LOG_DEFINE_APP_IDS("PELA", "Pelagicontain agent");

namespace pelagicontain {

using std::unique_ptr;

LOG_DECLARE_DEFAULT_CONTEXT(PAM_DefaultLogContext, "MAIN", "Main context");

class PelagicontainAgent
{

    typedef unique_ptr<PelagicontainLib> PelagicontainLibPtr;

public:
    PelagicontainAgent(Glib::RefPtr<Glib::MainContext> mainLoopContext
                     , int preloadCount
                     , bool shutdownContainers
                     , int shutdownTimeout)
        : m_mainLoopContext(mainLoopContext)
        , m_preloadCount(preloadCount)
        , m_shutdownContainers(shutdownContainers)
    {
        triggerPreload();
        m_pelagicontainWorkspace.m_containerShutdownTimeout = shutdownTimeout;
    }

    ~PelagicontainAgent()
    {
    }

    /**
     * Preload additional containers if needed
     */
    void triggerPreload()
    {
        //        log_debug() << "triggerPreload " << m_preloadCount - m_preloadedContainers.size();
        while (m_preloadedContainers.size() != m_preloadCount) {
            auto container = new PelagicontainLib(m_pelagicontainWorkspace);
            container->setContainerIDPrefix("Preload-");
            container->preload();
            m_preloadedContainers.push_back(PelagicontainLibPtr(container));
        }
    }

    void deleteContainer(ContainerID containerID)
    {
        bool valid = ((containerID < m_containers.size()) && (m_containers[containerID] != nullptr));
        if (valid) {
            m_containers[containerID] = nullptr;
        } else {
            log_error() << "Invalid container ID " << containerID;
        }

    }

    /**
     * Check whether the given containerID is valid and return a reference to the actual container
     */
    bool checkContainer(ContainerID containerID, PelagicontainLib * &container)
    {
        bool valid = ((containerID < m_containers.size()) && (m_containers[containerID] != nullptr));
        if (valid) {
            container = m_containers[containerID].get();
        } else {
            log_error() << "Invalid container ID " << containerID;
        }

        return valid;
    }

    /**
     * Create a new container
     */
    ContainerID createContainer(const std::string &prefix)
    {
        PelagicontainLib *container;
        if (m_preloadedContainers.size() != 0) {
            container = m_preloadedContainers[0].release();
            m_preloadedContainers.erase(m_preloadedContainers.begin());
        } else {
            container = new PelagicontainLib(m_pelagicontainWorkspace);
        }

        m_containers.push_back(PelagicontainLibPtr(container));
        auto id = m_containers.size() - 1;
        log_debug() << "Created container with ID :" << id;
        container->setContainerIDPrefix(prefix);
        container->setMainLoopContext(m_mainLoopContext);
        container->init();

        triggerPreload();

        return id;
    }

    bool checkJob(pid_t pid, CommandJob * &result)
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

    void writeToStdIn(pid_t pid, const std::vector<uint8_t> &bytes)
    {
        CommandJob *job = nullptr;
        if (checkJob(pid, job)) {
            log_debug() << "writing bytes to process with PID:" << job->pid() << " : " << bytes;
            write(job->stdin(), bytes.data(), bytes.size());
        }
    }

    /**
     * Launch the given command in a the given container
     */
    pid_t launchCommand(ContainerID containerID, uid_t userID, const std::string &cmdLine, const std::string &workingDirectory,
                const std::string &outputFile,
                const EnvironmentVariables &env, std::function<void (pid_t,
                        int)> listener)
    {
        PelagicontainLib *container;
        if (checkContainer(containerID, container)) {
            auto job = new CommandJob(*container, cmdLine);
            job->captureStdin();
            job->setOutputFile(outputFile);
            job->setUserID(userID);
            job->setEnvironnmentVariables(env);
            job->setWorkingDirectory(workingDirectory);
            job->start();
            addProcessListener(m_connections, job->pid(), [listener](pid_t pid, int exitCode) {
                            listener(pid, exitCode);
                        }, m_mainLoopContext);

            m_jobs.push_back(job);

            return job->pid();
        }
        return INVALID_PID;
    }

    void setContainerName(ContainerID containerID, const std::string &name)
    {
        PelagicontainLib *container = nullptr;
        if (checkContainer(containerID, container)) {
            container->setContainerName(name);
        }
    }

    void shutdownContainer(ContainerID containerID)
    {
        shutdownContainer(containerID, m_pelagicontainWorkspace.m_containerShutdownTimeout);
    }

    void shutdownContainer(ContainerID containerID, unsigned int timeout)
    {
        if (m_shutdownContainers) {
            PelagicontainLib *container = nullptr;
            if (checkContainer(containerID, container)) {
                container->shutdown();
                deleteContainer(containerID);
            }
        } else {
            log_info() << "Not shutting down container";
        }
    }

    std::string bindMountFolderInContainer(const uint32_t containerID, const std::string &pathInHost,
                const std::string &subPathInContainer, bool readOnly)
    {
        PelagicontainLib *container = nullptr;
        if (checkContainer(containerID, container)) {
            return container->getContainer().bindMountFolderInContainer(pathInHost, subPathInContainer, readOnly);
        }
        return "";
    }

    void setGatewayConfigs(const uint32_t &containerID, const std::map<std::string, std::string> &configs)
    {
        PelagicontainLib *container = nullptr;
        if (checkContainer(containerID, container)) {
            container->getPelagicontain().updateGatewayConfiguration(configs);
        }
    }

private:
    PelagicontainWorkspace m_pelagicontainWorkspace;
    std::vector<PelagicontainLibPtr> m_containers;
    std::vector<PelagicontainLibPtr> m_preloadedContainers;
    std::vector<CommandJob *> m_jobs;
    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    size_t m_preloadCount;
    SignalConnectionsHandler m_connections;
    bool m_shutdownContainers = true;
};

class PelagicontainAgentAdaptor :
    public com::pelagicore::PelagicontainAgent_adaptor
{

public:
    virtual ~PelagicontainAgentAdaptor()
    {
    }

    PelagicontainAgentAdaptor(PelagicontainAgent &agent) :
        m_agent(agent)
    {
    }

    uint32_t LaunchCommand(const uint32_t &containerID, const uint32_t &userID, const std::string &commandLine,
                const std::string &workingDirectory,
                const std::string &outputFile,
                const std::map<std::string,
                    std::string> &env)
    {
        return m_agent.launchCommand(containerID, userID, commandLine, workingDirectory, outputFile, env,
                    [this, containerID](pid_t pid, int exitCode) {
                        ProcessStateChanged(containerID, pid, false, exitCode);
                        log_info() << "ProcessStateChanged " << pid << " code " << exitCode;
                    });
    }

    void ShutDownContainerWithTimeout(const uint32_t &containerID, const uint32_t &timeout)
    {
        m_agent.shutdownContainer(containerID, timeout);
    }

    void ShutDownContainer(const uint32_t &containerID) override
    {
        m_agent.shutdownContainer(containerID);
    }

    std::string BindMountFolderInContainer(const uint32_t &containerID, const std::string &pathInHost,
                const std::string &subPathInContainer, const bool &readOnly) override
    {
        return m_agent.bindMountFolderInContainer(containerID, pathInHost, subPathInContainer, readOnly);
    }

    void SetGatewayConfigs(const uint32_t &containerID, const std::map<std::string, std::string> &configs) override
    {
        m_agent.setGatewayConfigs(containerID, configs);
    }

    uint32_t CreateContainer(const std::string &name) override
    {
        return m_agent.createContainer(name);
    }

    void SetContainerName(const uint32_t &containerID, const std::string &name) override
    {
        return m_agent.setContainerName(containerID, name);
    }

    void Ping() override
    {
    }

    void WriteToStdIn(const uint32_t &containerID, const std::vector<uint8_t> &bytes) override
    {
        m_agent.writeToStdIn(containerID, bytes);
    }

    PelagicontainAgent &m_agent;

};

}

int main(int argc, char * *argv)
{
    using ivi_main_loop::UNIXSignalHandler;

    pelagicore::CommandLineParser commandLineParser("Pelagicontain agent", "", PACKAGE_VERSION, "");

    int preloadCount = 0;
    commandLineParser.addOption(preloadCount, "preload", 'p', "Number of containers to preload");

    //    int userID = 0;
    //    commandLineParser.addOption(userID, "user", 'u', "Default user id to be used when starting processes in the container");

    bool shutdownContainers = true;
    commandLineParser.addOption(shutdownContainers, "shutdown", 's',
            "If false, the containers will not be shutdown. Useful for debugging");

    int timeout = 2;
    commandLineParser.addOption(timeout, "timeout", 't',
            "Timeout in seconds to wait for containers to shutdown");

    if (commandLineParser.parse(argc, argv)) {
        exit(1);
    }

    log_debug() << "Starting pelagicontain agent";

    auto mainContext = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> ml = Glib::MainLoop::create(mainContext);

    pelagicore::GLibDBusCppFactory glibDBusFactory(mainContext);

    // We try to use the system bus, and fallback to the session bus if the system bus can not be used
    auto connection = &glibDBusFactory.getSystemBusConnection();

    try {
        connection->request_name(AGENT_BUS_NAME);
    } catch (DBus::Error &error) {
        log_warning() << "Can't own the name" << AGENT_BUS_NAME << " on the system bus => use session bus instead";
        connection = &glibDBusFactory.getSessionBusConnection();
        connection->request_name(AGENT_BUS_NAME);
    }

    PelagicontainAgent agent(mainContext, preloadCount, shutdownContainers, timeout);

    auto pp = glibDBusFactory.registerAdapter<PelagicontainAgentAdaptor>(*connection, AGENT_OBJECT_PATH, agent);

    ivi_main_loop::GLibEventSourceManager eventSourceManager(mainContext->gobj());

    // Register UNIX signal handler
    auto signalHandler = [&] (int signal) {
        log_debug() << "caught signal " << signal;
        ml->quit();
    };
    UNIXSignalHandler handler(eventSourceManager, UNIXSignalHandler::HandlerMap {{SIGINT, signalHandler}, {SIGTERM, signalHandler}});
    handler.enable();

    ml->run();

    log_debug() << "Exiting pelagicontain agent";

    return 0;
}
