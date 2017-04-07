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

#pragma once

#include "observableproperty.h"
#include "gatewayconfig.h"
#include "signalconnectionshandler.h"
#include "config/softwarecontainerconfig.h"
#include "filetoolkitwithundo.h"
#include "softwarecontainererror.h"
#include "softwarecontainerabstractinterface.h"

#include <glibmm.h>

#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "functionjob.h"
#include "commandjob.h"

namespace softwarecontainer {

class Gateway;
class ContainerAbstractInterface;


enum class ContainerState
{
    READY,
    SUSPENDED,
    TERMINATED,
    INVALID
};


/**
 * @class InvalidOperationError
 *
 * @brief A method was called which is inappropriate in the current state.
 */
class InvalidOperationError : public SoftwareContainerError
{
public:
    InvalidOperationError():
        SoftwareContainerError("Invalid operation on container")
    {
    }

    InvalidOperationError(const std::string &message):
        SoftwareContainerError(message)
    {
    }
};


/**
 * @class ContainerError
 *
 * @brief An error has occured in the underlying container implementation
 */
class ContainerError : public SoftwareContainerError
{
public:
    ContainerError():
        SoftwareContainerError("Error in container implementation")
    {
    }

    ContainerError(const std::string &message):
        SoftwareContainerError(message)
    {
    }
};


/**
 * @class InvalidContainerError
 *
 * @brief The container instance is in an invalid state and should not be used
 */
class InvalidContainerError : public SoftwareContainerError
{
public:
    InvalidContainerError():
        SoftwareContainerError("Container is in an invalid state")
    {
    }

    InvalidContainerError(const std::string &message):
        SoftwareContainerError(message)
    {
    }
};


/**
 * @class SoftwareContainer
 *
 * @brief An abstraction of concrete container implementations
 *
 * SoftwareContainer (SC) can be in various states. Some method calls are only valid
 * when SC is in a particular state.
 *
 * States:
 *  * READY
 *  * SUSPENDED
 *  * TERMINATED
 *  * INVALID
 *
 * When SC is in state 'READY', all relevant gateways are added and the underlying
 * container implementation is created and initialized.  In this state,
 * the following methods are allowed to be called:
 *
 *  * suspend() - successful call triggers transition to SUSPENDED
 *  * shutdown() - successful call triggers trasition to TERMINATED
 *  * bindMount()
 *  * startGateways()
 *  * createFunctionJob()
 *  * createCommandJob()
 *
 * When SC is in state 'SUSPENDED', the following method calls are allowed to be
 * called:
 *
 *  * shutdown() - successful call triggers a transition to TERMINATED
 *  * resume() - successful call triggers a transition to READY
 *
 * When SC is in state 'INVALID', something has gone wrong in the underlying container
 * implementation and the container should be considered to be in a broken state from
 * which it cannot recover.
 *
 * The following methods are allowed to be called in every state:
 *
 *  * getContainerState()
 *
 * If a call is made when SC is in an inappropriate state, the InvalidOperationError
 * exception is thrown. This error does not mean that the intended operation
 * was erroneous, since the operation can't even be performed in the current state.
 *
 * It is an error to call a method on a state which would be the result of the
 * operation, e.g. calling suspend on a suspended container is an invalid operation.
 * The reason for this is that the interface should be clear that there can't be
 * any side effects or other changes when making a call which is redundant.
 */
class SoftwareContainer :
    private FileToolkitWithUndo,
    public SoftwareContainerAbstractInterface
{
public:
    LOG_DECLARE_CLASS_CONTEXT("PCL", "SoftwareContainer library");

    /**
     * @brief Creates a new SoftwareContainer instance.
     *
     * On successful creation, the state will be 'READY'.
     *
     * @param id The containerID to use.
     * @param config An object holding all needed settings for the container
     *
     * @throws SoftwareContainerError If unable to set up the needed directories
     *         or network settings for this container, or if anything goes wrong
     *         when creating and initializing the underlying container implementation.
     */
    SoftwareContainer(const ContainerID id,
                      std::unique_ptr<const SoftwareContainerConfig> config);

    ~SoftwareContainer();

    /**
     * @brief Starts the Gateways by setting the gateway configurations
     * and activating the configured gateway
     *
     * This should only be called on containers in state 'READY'
     *
     * @return true If configuration and activation was successful
     * @return false If configuration or activation encountered
     *         a non-fatal error
     *
     * @throws GatewayError If configuration or activation of any gateway
     *                      encountered a fatal error.
     * @throws InvalidOperationError If called when state is not 'READY'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    bool startGateways(const GatewayConfiguration &configs);

    /**
     * @brief Create a job that can run a function in a container
     *
     * This should only be called on containers in state 'READY'
     *
     * @throws InvalidOperationError If called when state is not 'READY'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    std::shared_ptr<FunctionJob> createFunctionJob(const std::function<int()> fun);

    /**
     * @brief Create a job that can run a command in a container
     *
     * This should only be called on containers in state 'READY'
     *
     * @throws InvalidOperationError If called when state is not 'READY'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    std::shared_ptr<CommandJob> createCommandJob(const std::string &command);

    /**
     * @brief Shut down the container
     *
     * A successful call to this method triggers a transition to state
     * 'TERMINATED'.
     *
     * This shoud only be called on containers in state 'READY' or 'SUSPENDED'
     *
     * If the operation is not successful, the container state will be set to
     * 'INVALID'. This means that this container instance should be considered
     * broken.
     *
     * @throws ContainerError If operation was unsucessful
     * @htrows InvalidOperationError If called when state is not 'READY' or
     *         'SUSPENDED'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    void shutdown();

    /**
     * @brief Shot down the container with an explicit timeout
     *
     * Same as the shutdown() method, but the value passed will override any
     * timeout value in the main configuration.
     *
     * @param timeout Seconds to use for timeout
     */
    void shutdown(unsigned int timeout);

    /**
     * @brief Suspend the container
     *
     * This suspends any execution inside the container until resume is called.
     * A successful call to this method triggers a transition to state
     * 'SUSPENDED'.
     *
     * If the operation is not successful, the container state will be set to
     * 'INVALID'. This means that this container instance should be considered
     * broken.
     *
     * @throws ContainerError If operation was unsuccessful
     * @throws InvalidOperationError If called when state is not 'READY'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    void suspend();

    /**
     * @brief Resume a suspended container
     *
     * This resumes execution of a container that was suspended. A successful
     * call to this method will trigger a transition to state 'READY'.
     *
     * If the operation is not successful, the container state will be set to
     * 'INVALID'. This means that this container instance should be considered
     * broken.
     *
     * @throws ContainerError If operation was unsucessful
     * @throws InvalidOperationError If called when state is not 'SUSPENDED'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    void resume();

    /**
     * Should only be called on containers in state 'READY'
     *
     * @throws InvalidOperationError If called when state is not 'READY'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    bool bindMount(const std::string &pathOnHost,
                         const std::string &pathInContainer,
                         bool readonly = true);

    /**
     * @brief Get the state of this container instance
     *
     * @return ContainerState representing the current state
     */
    ObservableProperty<ContainerState> &getContainerState();

    /**
     * @brief Indicates if gateways have been configured previously.
     *
     * The user should use this method to check if startGateways have
     * been called previously.
     *
     * TODO: This should be removed when quick-launch is implemented
     *
     * @return true if startGateways has been called previously, false if not
     */
    bool previouslyConfigured();

private:
    /*
     * Add gateways and create and initialize the underlying container
     * implementation.
     */
    bool init();

    bool start();

    bool configureGateways(const GatewayConfiguration &gwConfig);
    bool activateGateways();
    bool shutdownGateways();

    std::string getContainerDir();
    std::string getGatewayDir();

    void assertValidState();

    // Check if the workspace is sound, and set it up if it isn't
    void checkContainerRoot(std::string rootDir);
#ifdef ENABLE_NETWORKGATEWAY
    void checkNetworkSettings();
#endif // ENABLE_NETWORKGATEWAY

    ContainerID m_containerID;
    std::shared_ptr<ContainerAbstractInterface> m_container;
    pid_t m_pcPid = INVALID_PID;
    ObservableWritableProperty<ContainerState> m_containerState;

    std::vector<std::unique_ptr<Gateway>> m_gateways;
    std::unique_ptr<const SoftwareContainerConfig> m_config;

    std::string m_containerRoot;
    unsigned int m_tmpfsSize;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    SignalConnectionsHandler m_connections;

    // Keeps track of if startGateways has been called on this instance
    bool m_previouslyConfigured;
};

} // namespace softwarecontainer
