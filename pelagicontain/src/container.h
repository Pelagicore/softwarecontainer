/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTAINER_H
#define CONTAINER_H

#include <string>
#include <vector>
#include <sys/mount.h>

#include "pelagicontain-common.h"

/*! Container is an abstraction of the specific containment technology used.
 *
 * The Container class is meant to be an abstraction of the containment
 * technology, so that Pelagicontain can view it as a generic concept that
 * implements the specifics behind the conceptual phases of 'Pereload', 'Launch',
 * and 'Shutdown'.
 */
class Container
{
    LOG_DECLARE_CLASS_CONTEXT("CONT", "Container");

    static constexpr const char *GATEWAYS_PATH = "/gateways";
    static constexpr const char *LATE_MOUNT_PATH = "/late_mounts";

    enum class LXCContainerState
    {
        STOPPED, STARTING, RUNNING, STOPPING, ABORTING, FREEZING, FROZEN, THAWED, ELEMENT_COUNT
    };

    static std::vector<const char *> s_LXCContainerStates;

    static void init_lxc();

    static const char *toString(LXCContainerState state)
    {
        return s_LXCContainerStates[static_cast<int>(state)];
    }

public:
    static constexpr uid_t ROOT_UID = 0;

    class CleanUpHandler
    {
protected:
        LOG_SET_CLASS_CONTEXT( Container::getDefaultContext() );
public:
        virtual ~CleanUpHandler()
        {
        }
        virtual ReturnCode clean() = 0;
    };

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
    Container(const std::string &id, const std::string &name, const std::string &configFile, const std::string &containerRoot);

    ~Container();

    /*!
     * Calls the lxc-create command.
     */
    void create();

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

    pid_t executeInContainer(ContainerFunction function,
            const EnvironmentVariables &variables = EnvironmentVariables(), uid_t userID = ROOT_UID, int stdin = -1, int stdout =
                1,
            int stderr = 2);

    std::string bindMountFileInContainer(const std::string &src, const std::string &dst, bool readonly = true);

    std::string bindMountFolderInContainer(const std::string &src, const std::string &dst, bool readonly = true);

    ReturnCode mountDevice(const std::string &pathInHost);

    /**
     * Old style mount. Used by headless launcher
     */
    bool mountApplication(const std::string &appDirBase);

    /*!
     * Calls the lxc-destroy command.
     */
    void destroy();

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
     * setApplication is called. If the 'late_mount' directory is missing
     * or any subddirectory to it could not be created, \c false is returned,
     * if all went well, \c true is returned.
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
        // TODO: see how to put gatewaysDir into the late_mounts to save one mount
        //      return applicationMountDir() + GATEWAYS_PATH;
        return m_containerRoot + "/" + id() + GATEWAYS_PATH;
    }

    std::string lateMountDir() const
    {
        return m_containerRoot + LATE_MOUNT_PATH;
    }

    std::string applicationMountDir() const
    {
        return lateMountDir() + "/" + m_id;
    }

    const std::string &root() const
    {
        return m_containerRoot;
    }

    ReturnCode setEnvironmentVariable(const std::string &var, const std::string &val);

    ReturnCode systemCall(const std::string &cmd);

private:
    static int executeInContainerEntryFunction(void *param);

    /**
     * Create a directory, and if successful append it to a list of dirs
     * to be deleted in the dtor. Since nestled dirs will need to be
     * deleted in reverse order to creation insert to the beginning of
     * the list.
     */
    ReturnCode createDirectory(const std::string &path);

    /**
     * Create a bind mount. On success the mount will be added to a list of
     * mounts that will be unmounted in the dtor.
     */
    ReturnCode bindMount(const std::string &src, const std::string &dst, bool readOnly = true);

    std::vector<CleanUpHandler *> m_cleanupHandlers;

    /**
     * The LXC configuration file for this container
     */
    std::string m_configFile;

    /**
     * The unique name of the LXC container
     */
    const std::string &m_id;

    /**
     * The name assigned to the container
     */
    const std::string &m_name;

    struct lxc_container *m_container = nullptr;

    std::string m_containerRoot;

    EnvironmentVariables m_gatewayEnvironmentVariables;

    bool m_initialized = false;
};

#endif //CONTAINER_H
