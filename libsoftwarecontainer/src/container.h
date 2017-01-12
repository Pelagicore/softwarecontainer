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

#pragma once

#include <string>
#include <vector>

#include <lxc/lxccontainer.h>

#include "filetoolkitwithundo.h"

#include "softwarecontainer-common.h"
#include "containerabstractinterface.h"

namespace softwarecontainer {

/**
 * @brief The Container class is an abstraction of the specific containment technology used.
 *
 * The Container class is meant to be an abstraction of the containment
 * technology, so that SoftwareContainer can view it as a generic concept that
 * implements the specifics behind the conceptual phases of 'Pereload', 'Launch',
 * and 'Shutdown'.
 */
class Container :
    private FileToolkitWithUndo, public ContainerAbstractInterface
{
    LOG_DECLARE_CLASS_CONTEXT("CONT", "Container");

    static constexpr const char *GATEWAYS_PATH = "/gateways";

    enum class LXCContainerState
    {
        STOPPED, STARTING, RUNNING, STOPPING, ABORTING, FREEZING, FROZEN, THAWED, ELEMENT_COUNT
    };

    static const char *toString(LXCContainerState state)
    {
        return s_LXCContainerStates[static_cast<int>(state)];
    }

    static std::vector<const char *> s_LXCContainerStates;
    static const char *s_LXCRoot;

    /**
     * @brief Loads states from LXC to check what state we are in
     *
     * This needs to be run once before creating containers.
     */
    static void init_lxc();

public:

    /**
     * @brief Constructor
     *
     * @param name Name of the container
     * @param configFile Path to the configuration file (including the file name)
     * @param containerRoot A path to the root of the container, i.e. the base
     *  path to e.g. the configurations and application root
     * @param enableWriteBuffer Enable RAM write buffers on top of rootfs
     * @param shutdownTimeout Timeout for shutdown of container.
     */
    Container(const std::string id
            , const std::string &configFile
            , const std::string &containerRoot
            , bool enableWriteBuffer = false
            , int shutdownTimeout = 1);

    ~Container();

    /**
     * @brief create Creates a new lxc_container and creates it with all the initialization.
     * @return ReturnCode::FAILURE on failed creation and ReturnCode::SUCCESS on a successful
     *  creation.
     */
    ReturnCode create();

    /**
     * @brief Start the container
     * @return The pid of the init process of the container
     */
    ReturnCode start(pid_t *pid);

    ReturnCode setCgroupItem(std::string subsys, std::string value);

    /**
     * @brief Start a process from the given command line, with an environment consisting of the
     * variables previously set by the gateways,
     * plus the ones passed as parameters here.
     */
    ReturnCode execute(const std::string &commandLine,
                       pid_t *pid,
                       const EnvironmentVariables &variables,
                       const std::string &workingDirectory = "/",
                       int stdin = -1,
                       int stdout = 1,
                       int stderr = 2);

    ReturnCode execute(ExecFunction function,
                       pid_t *pid,
                       const EnvironmentVariables &variables = EnvironmentVariables(),
                       int stdin = -1,
                       int stdout = 1,
                       int stderr = 2);


    /**
     * @brief Tries to bind mount a path from host to container
     *
     * Any missing parent paths will be created.
     *
     * @param pathInHost The path on the host that shall be bind mounted into the container
     * @param pathInContainer Where to mount the path in the container.
     * @param readonly Sets if the mount should be read only or read write
     *
     * @return SUCCESS if everything worked as expected, FAILURE otherwise
     */
    ReturnCode bindMountInContainer(const std::string &pathInHost,
                                    const std::string &pathInContainer,
                                    bool readOnly = true);


    ReturnCode mountDevice(const std::string &pathInHost);

    ReturnCode createSymLink(const std::string &source, const std::string &destination)
    {
        return FileToolkitWithUndo::createSymLink(source, destination);
    }

    /**
     * @brief Calls shutdown, and then destroys the container
     */
    ReturnCode destroy();
    ReturnCode destroy(unsigned int timeout);

    /**
     * @brief Calls shutdown on the lxc container
     */
    ReturnCode shutdown();
    ReturnCode shutdown(unsigned int timeout);

    /*
     * @brief calls freeze() on the LXC container
     *
     * This only works if the container is currently running and is not already
     * suspended.
     *
     * @return ReturnCode::SUCCESS if the container was successfully suspended
     * @return ReturnCode::FAILURE otherwise
     */
    ReturnCode suspend();

    /*
     * @brief calls unfreeze() on the LXC container
     *
     * This only works if the container was already suspended. This sets the container
     * into running state again.
     *
     * @return ReturnCode::SUCCESS if the container was successfully resumed
     * @return ReturnCode::FAILURE otherwise
     */
    ReturnCode resume();

    /**
     * @brief Calls stop on the lxc container(force stop)
     */
    ReturnCode stop();

    ReturnCode waitForState(LXCContainerState state, int timeout = 20);
    ReturnCode ensureContainerRunning();

    /**
     * @brief Setup the container for preloading
     *
     * Setup the container so directories are available for later use when
     * setApplication is called.
     * @return true or false
     */
    ReturnCode initialize();

    std::string toString();

    const char *id() const;
    std::string gatewaysDirInContainer() const;
    std::string gatewaysDir() const;
    const std::string &rootFS() const;

    ReturnCode setEnvironmentVariable(const std::string &var, const std::string &val);

private:
    static int executeInContainerEntryFunction(void *param);

    /**
     * @brief Tries to bind mount a file in the container
     *
     * Any missing parent paths will be created.
     *
     * @param pathInHost The path to the file that shall be bind mounted on the host system.
     * @param pathInContainer Where to mount the file in the container.
     * @param readonly Sets if the mount should be read only or read write
     *
     * @return SUCCESS if everything worked as expected, FAILURE otherwise.
     */
    ReturnCode bindMountFileInContainer(const std::string &pathInHost,
                                        const std::string &pathInContainer,
                                        const std::string &tempFile,
                                        bool readonly = true);

    /**
     * @brief Tries to bind mount a directory in the container
     *
     * Any missing parent paths will be created.
     *
     * @param pathInHost The path to the directory that shall be bind mounted on the host system.
     * @param pathInContainer Where to mount the file in the container.
     * @param readonly Sets if the mount should be read only or read write
     *
     * @return SUCCESS if everything worked as expected,FAILURE otherwise.
     */
    ReturnCode bindMountDirectoryInContainer(const std::string &pathInHost,
                                             const std::string &pathInContainer,
                                             const std::string &tempDir,
                                             bool readonly = true);

    ReturnCode bindMountCore(const std::string &pathInHost,
                             const std::string &pathInContainer,
                             const std::string &tempDir,
                             bool readonly);

    ReturnCode remountReadOnlyInContainer(const std::string &path);

    /**
     * @brief Helper function that rollsback the changes done in Container::create().
     */
    ReturnCode rollbackCreate();

    /**
     * @brief The LXC configuration file for this container
     */
    std::string m_configFile;

    /**
     * @brief The unique name of the LXC container
     */
    const std::string m_id;

    std::string m_rootFSPath;

    /**
     * @brief Pointer to the LXC container
     */
    struct lxc_container *m_container = nullptr;

    std::string m_containerRoot;

    bool m_enableWriteBuffer;

    // All environment variables set by gateways
    EnvironmentVariables m_gatewayEnvironmentVariables;

    int m_shutdownTimeout = 1;

    enum class ContainerState : unsigned int {
        DEFAULT = 0,
        PREPARED = 1,
        DESTROYED = 2,
        CREATED = 3,
        STARTED = 4,
        FROZEN = 5,
    };
    ContainerState m_state = ContainerState::DEFAULT;
};

} // namespace softwarecontainer
