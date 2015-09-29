#pragma once

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <glibmm.h>

#include "pelagicontain.h"
#include "gateway.h"

class JobAbstract;

namespace pelagicontain {

class PelagicontainWorkspace :
    private FileToolkitWithUndo
{

    LOG_DECLARE_CLASS_CONTEXT("PCLW", "Pelagicontain library workspace");

public:
    PelagicontainWorkspace(
            const std::string &containerRootFolder = PELAGICONTAIN_DEFAULT_WORKSPACE,
            const std::string &configFilePath = PELAGICONTAIN_DEFAULT_CONFIG,
            unsigned int containerShutdownTimeout = 2)
        : m_containerRoot(containerRootFolder)
        , m_containerConfig(configFilePath)
        , m_containerShutdownTimeout(containerShutdownTimeout)
    {
        // Make sure path ends in '/' since it might not always be checked
        if (m_containerRoot.back() != '/') {
            m_containerRoot += "/";
        }

        if (isError(checkWorkspace())) {
            log_error() << "Failed when checking workspace";
            assert(false);
        }

    }

    ~PelagicontainWorkspace()
    {
        deleteWorkspace();
    }

    ReturnCode deleteWorkspace();

    /**
     * Check if the workspace is present and create it if needed
     */
    ReturnCode checkWorkspace();

    std::string m_containerRoot;
    std::string m_containerConfig;
    unsigned int m_containerShutdownTimeout;

};

PelagicontainWorkspace &getDefaultWorkspace();

class PelagicontainLib :
    private FileToolkitWithUndo
{
public:
    LOG_DECLARE_CLASS_CONTEXT("PCL", "Pelagicontain library");

    PelagicontainLib(PelagicontainWorkspace &workspace = getDefaultWorkspace());

    ~PelagicontainLib();

    /**
     * Set the main loop
     */
    void setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext)
    {
        m_ml = mainLoopContext;
    }

    void shutdown()
    {
        m_pelagicontain.shutdown();
    }

    void shutdown(unsigned int timeout)
    {
        m_pelagicontain.shutdown(timeout);
    }

    bool isInitialized() const
    {
        return m_initialized;
    }

    void openTerminal(const std::string &terminalCommand) const;

    /**
     * Preload the container.
     * That method can be called before setting the main loop context
     */
    ReturnCode preload();

    ReturnCode init();

    void setGatewayConfigs(const GatewayConfiguration &config)
    {
        m_pelagicontain.updateGatewayConfiguration(config);
    }

    const Container &getContainer() const
    {
        return m_container;
    }

    Container &getContainer()
    {
        return m_container;
    }

    Pelagicontain &getPelagicontain()
    {
        return m_pelagicontain;
    }

    std::string getContainerDir()
    {
        return m_workspace.m_containerRoot + "/" + getContainerID();
    }

    std::string getGatewayDir()
    {
        return getContainerDir() + "/gateways";
    }

    void setContainerIDPrefix(const std::string &prefix);

    void setContainerName(const std::string &name);

    const std::string &getContainerID()
    {
        return m_containerID;
    }

    void validateContainerID();

private:
    PelagicontainWorkspace &m_workspace;

    std::string m_containerID;
    std::string m_containerName;

    Container m_container;

    Glib::RefPtr<Glib::MainContext> m_ml;

    Pelagicontain m_pelagicontain;

    std::vector<std::unique_ptr<Gateway> > m_gateways;

    SignalConnectionsHandler m_connections;

    pid_t m_pcPid = 0;

    bool m_initialized = false;

};


/**
 * Abstract class for jobs which get executed inside a container
 */
class JobAbstract
{
protected:
    LOG_SET_CLASS_CONTEXT(PelagicontainLib::getDefaultContext());

public:
    static constexpr int UNASSIGNED_STREAM = -1;

    JobAbstract(PelagicontainLib &lib) :
        m_lib(lib)
    {
    }

    virtual ~JobAbstract()
    {
    }

    void captureStdin()
    {
        pipe(m_stdin);
    }

    void setOutputFile(const std::string &path)
    {
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        m_stdout[1] = m_stderr[1] = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
        log_debug() << "stdout/stderr redirected to " << path << " fd:" << m_stdout[0];
    }

    void captureStdout()
    {
        pipe(m_stdout);
    }

    void captureStderr()
    {
        pipe(m_stderr);
    }

    int wait()
    {
        return waitForProcessTermination(m_pid);
    }

    int stdout()
    {
        return m_stdout[0];
    }

    int stderr()
    {
        return m_stderr[0];
    }

    int stdin()
    {
        return m_stdin[1];
    }

    pid_t pid()
    {
        return m_pid;
    }

    /**
     * That method always returns true as soon as the start() method has been called, even if the command fails to start,
     * since we don't know if the exec() occurring after the fork into the container actually succeeds...
     */
    bool isRunning()
    {
        // TODO : find a way to test whether the exec() which occurs in the container succeeded
        return (m_pid != 0);
    }

    void setEnvironnmentVariable(const std::string &key, const std::string &value)
    {
        m_env[key] = value;
    }

    void setEnvironnmentVariables(const EnvironmentVariables &env)
    {
        m_env = env;
    }

    Container &getContainer()
    {
        return m_lib.getContainer();
    }

protected:
    EnvironmentVariables m_env;
    PelagicontainLib &m_lib;
    pid_t m_pid = 0;
    int m_stdin[2] = {UNASSIGNED_STREAM, UNASSIGNED_STREAM};
    int m_stdout[2] = {UNASSIGNED_STREAM, UNASSIGNED_STREAM};
    int m_stderr[2] = {UNASSIGNED_STREAM, UNASSIGNED_STREAM};
};

/**
 * Use this class to execute a command line in a container
 */
class CommandJob :
    public JobAbstract
{

public:
    static constexpr int UNASSIGNED_STREAM = -1;

    CommandJob(PelagicontainLib &lib, const std::string &command) :
        JobAbstract(lib)
    {
        m_command = command;
    }

    ReturnCode setWorkingDirectory(const std::string &folder)
    {
        m_workingDirectory = folder;
        return ReturnCode::SUCCESS;
    }

    ReturnCode setUserID(uid_t userID)
    {
        m_userID = userID;
        return ReturnCode::SUCCESS;
    }

    ReturnCode start()
    {
        m_pid = getContainer().attach(m_command, m_env, m_userID, m_workingDirectory, m_stdin[0], m_stdout[1], m_stderr[1]);
        return (m_pid != 0) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
    }

    std::string toString() const
    {
        return logging::StringBuilder() << "Pelagicontain job. command: " << m_command << " stdin:" << m_stdin[0]
                                        << " stdout:" << m_stdout[1];
    }

private:
    std::string m_command;
    std::string m_workingDirectory;
    uid_t m_userID = ROOT_UID;
};

/**
 *
 */
class FunctionJob :
    public JobAbstract
{

public:
    static constexpr int UNASSIGNED_STREAM = -1;

    FunctionJob(PelagicontainLib &lib, std::function<int()> command) :
        JobAbstract(lib)
    {
        m_command = command;
    }

    ReturnCode start()
    {
        m_pid = getContainer().executeInContainer(m_command, m_env, m_stdin[0], m_stdout[1], m_stderr[1]);
        return (m_pid != 0) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
    }

    void setEnvironnmentVariable(const std::string &key, const std::string &value)
    {
        m_env[key] = value;
    }

    std::string toString() const
    {
        return logging::StringBuilder() << "Pelagicontain job. " << " stdin:" << m_stdin[0]
                                        << " stdout:" << m_stdout[1];
    }

private:
    std::function<int()> m_command;
};

}
