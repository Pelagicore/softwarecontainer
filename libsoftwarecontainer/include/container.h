/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTAINER_H
#define CONTAINER_H

#include <string>
#include <vector>

#include <lxc/lxccontainer.h>

#include "softwarecontainer-common.h"



/*! Container is an abstraction of the specific containment technology used.
 *
 * The Container class is meant to be an abstraction of the containment
 * technology, so that SoftwareContainer can view it as a generic concept that
 * implements the specifics behind the conceptual phases of 'Pereload', 'Launch',
 * and 'Shutdown'.
 */
class Container :
    private FileToolkitWithUndo
{
    LOG_DECLARE_CLASS_CONTEXT("CONT", "Container");

    static constexpr const char *GATEWAYS_PATH = "/gateways";

    enum class LXCContainerState
    {
        STOPPED, STARTING, RUNNING, STOPPING, ABORTING, FREEZING, FROZEN, THAWED, ELEMENT_COUNT
    };

    static std::vector<const char *> s_LXCContainerStates;
    static const char *s_LXCRoot;

    static void init_lxc();

    static const char *toString(LXCContainerState state)
    {
        return s_LXCContainerStates[static_cast<int>(state)];
    }

public:
    /// A function to be executed in the container
    typedef std::function<int ()> ContainerFunction;

    /*!
     * Constructor
     *
     * \param name Name of the container
     * \param configFile Path to the configuration file (including the file name)
     * \param containerRoot A path to the root of the container, i.e. the base
     *  path to e.g. the configurations and application root
     * \param containedCommand The command to be executed inside the container
     */
    Container(const std::string &id, const std::string &name, const std::string &configFile, const std::string &containerRoot, int shutdownTimeout = 2);

    ~Container();

    /*!
     * Calls the lxc-create command.
     */
    ReturnCode create();

    /*!
     * Start the container
     *
     * \return The pid of the init process of the container
     */
    pid_t start();

    /**
     * Start a process from the given command line, with an environment consisting of the variables previously set by the gateways,
     * plus the ones passed as parameters here.
     */
    pid_t attach(const std::string &commandLine, const EnvironmentVariables &variables, uid_t userID,
            const std::string &workingDirectory = "/", int stdin = -1, int stdout = 1,
            int stderr = 2);

    /**
     * Start a process with the environment variables which have previously been set by the gateways
     */
    pid_t attach(const std::string &commandLine, uid_t userID = ROOT_UID);

    ReturnCode setCgroupItem(std::string subsys, std::string value);

    ReturnCode setUser(uid_t userID);

    pid_t executeInContainer(ContainerFunction function,
            const EnvironmentVariables &variables = EnvironmentVariables(), uid_t userID = ROOT_UID, int stdin = -1, int stdout =
                1, int stderr = 2);

    std::string bindMountFileInContainer(const std::string &src, const std::string &dst, bool readonly = true);

    std::string bindMountFolderInContainer(const std::string &src, const std::string &dst, bool readonly = true);

    ReturnCode mountDevice(const std::string &pathInHost);

    ReturnCode createSymLink(const std::string &source, const std::string &destination)
    {
        return FileToolkitWithUndo::createSymLink(source, destination);
    }

    /*!
     * Calls the lxc-destroy command.
     */
    void destroy();
    void destroy(unsigned int timeout);

    void stop();

    void waitForState(LXCContainerState state, int timeout = 20);

    void ensureContainerRunning()
    {
        waitForState(LXCContainerState::RUNNING);
    }

    /*!
     * Setup the container for preloading
     *
     * Setup the container so directories are available for later use when
     * setApplication is called.
     * If all went well, \c true is returned, \c false otherwise
     *
     * \return true or false
     */
    ReturnCode initialize();

    bool isInitialized()
    {
        return m_initialized;
    }

    std::string toString();

    const char *id() const
    {
        return m_id.c_str();
    }

    std::string gatewaysDirInContainer() const
    {
        return GATEWAYS_PATH;
    }

    std::string gatewaysDir() const
    {
        return m_containerRoot + "/" + id() + GATEWAYS_PATH;
    }

    const std::string &root() const
    {
        return m_containerRoot;
    }

    const std::string &rootFS() const
    {
        return m_rootFSPath;
    };

    ReturnCode setEnvironmentVariable(const std::string &var, const std::string &val);

    ReturnCode executeInContainer(const std::string &cmd);

private:
    static int executeInContainerEntryFunction(void *param);

    /**
     * The LXC configuration file for this container
     */
    std::string m_configFile;

    /**
     * The unique name of the LXC container
     */
    const std::string &m_id;

    std::string m_rootFSPath;

    /**
     * The name assigned to the container
     */
    const std::string &m_name;

    struct lxc_container *m_container = nullptr;

    std::string m_containerRoot;

    EnvironmentVariables m_gatewayEnvironmentVariables;

    bool m_initialized = false;
    bool m_created = false;

    int m_shutdownTimeout = 2;
};

#endif //CONTAINER_H
