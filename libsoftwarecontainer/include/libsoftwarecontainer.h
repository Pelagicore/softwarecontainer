#pragma once

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <glibmm.h>

#include "gateway.h"

namespace softwarecontainer {

class SoftwareContainerWorkspace :
    private FileToolkitWithUndo
{
    LOG_DECLARE_CLASS_CONTEXT("PCLW", "SoftwareContainer library workspace");

public:
    SoftwareContainerWorkspace(
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

    ~SoftwareContainerWorkspace()
    {
    }

    /**
     * Check if the workspace is present and create it if needed
     */
    ReturnCode checkWorkspace();

    std::string m_containerRoot;
    std::string m_containerConfig;
    unsigned int m_containerShutdownTimeout;
};

SoftwareContainerWorkspace &getDefaultWorkspace();

class SoftwareContainerLib :
    private FileToolkitWithUndo
{
public:
    LOG_DECLARE_CLASS_CONTEXT("PCL", "SoftwareContainer library");

    SoftwareContainerLib(SoftwareContainerWorkspace &workspace = getDefaultWorkspace());

    ~SoftwareContainerLib();

    /**
     * Set the main loop
     */
    void setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext);

    /* 
     * Shutdown the container
     */ 
    ReturnCode shutdown();
    ReturnCode shutdown(unsigned int timeout);

    bool isInitialized() const;

    void addGateway(Gateway *gateway);

    void openTerminal(const std::string &terminalCommand) const;

    /**
     * Preload the container.
     * That method can be called before setting the main loop context
     */
    ReturnCode preload();
    ReturnCode init();

    std::shared_ptr<ContainerAbstractInterface> getContainer();
    const std::string &getContainerID();
    std::string getContainerDir();
    std::string getGatewayDir();

    void setContainerIDPrefix(const std::string &prefix);
    void setContainerName(const std::string &name);

    void validateContainerID();

    ObservableProperty<ContainerState> &getContainerState();

    pid_t launchCommand(const std::string &commandLine);

    /*! Continues the 'launch' phase by allowing gateway configurations to
     *  be set.
     *
     * Platform Access Manager calls this method after SoftwareContainer has
     * registered as a client, and passes all gateway configurations as
     * argument. SoftwareContainer sets the gateway configurations and activates
     * all gateways as a result of this call. The contained application
     * is then started.
     *
     * \param configs A map of gateway IDs and their respective configurations
     */
    void updateGatewayConfiguration(const GatewayConfiguration &configs);

    /**
     * Set the gateway configuration and activate them
     */
    void setGatewayConfigs(const GatewayConfiguration &configs);

private:
    ReturnCode shutdownGateways();

    SoftwareContainerWorkspace &m_workspace;

    std::string m_containerID;
    std::string m_containerName;
    ObservableWritableProperty<ContainerState> m_containerState;

    std::shared_ptr<ContainerAbstractInterface> m_container;
    pid_t m_pcPid = INVALID_PID;
    std::vector<std::unique_ptr<Gateway> > m_gateways;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    SignalConnectionsHandler m_connections;

    bool m_initialized = false;
};


/**
 * Abstract class for jobs which get executed inside a container
 */
class JobAbstract
{
protected:
    LOG_SET_CLASS_CONTEXT(SoftwareContainerLib::getDefaultContext());

public:
    static constexpr int UNASSIGNED_STREAM = -1;

    JobAbstract(SoftwareContainerLib &lib) :
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

    std::shared_ptr<ContainerAbstractInterface> getContainer()
    {
        return m_lib.getContainer();
    }

protected:
    EnvironmentVariables m_env;
    SoftwareContainerLib &m_lib;
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

    CommandJob(SoftwareContainerLib &lib, const std::string &command) :
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
        return getContainer()->attach(m_command, &m_pid, m_env, m_userID, m_workingDirectory,
                                      m_stdin[0], m_stdout[1], m_stderr[1]);
    }

    std::string toString() const
    {
        return logging::StringBuilder() << "SoftwareContainer job. command: "
                                        << m_command << " stdin:" << m_stdin[0]
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

    FunctionJob(SoftwareContainerLib &lib, std::function<int()> command) :
        JobAbstract(lib)
    {
        m_command = command;
    }

    ReturnCode start()
    {
        return getContainer()->executeInContainer(m_command, &m_pid, m_env, m_stdin[0], m_stdout[1], m_stderr[1]);
    }

    void setEnvironnmentVariable(const std::string &key, const std::string &value)
    {
        m_env[key] = value;
    }

    std::string toString() const
    {
        return logging::StringBuilder() << "SoftwareContainer job. " << " stdin:" << m_stdin[0]
                                        << " stdout:" << m_stdout[1];
    }

private:
    std::function<int()> m_command;
};

}
