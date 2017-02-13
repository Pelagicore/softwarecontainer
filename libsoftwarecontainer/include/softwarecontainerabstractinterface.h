/*
 * Copyright (C) 2017 Pelagicore AB
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

/**
 * @file softwarecontainerabstractinterface.h
 * @brief Contains the softwarecontainer::SoftwareContainerAbstractInterface class
 */
#pragma once

#include "commandjob.h"
#include "gatewayconfig.h"

namespace softwarecontainer {

class SoftwareContainerAbstractInterface
{
public:

    virtual ~SoftwareContainerAbstractInterface() {};

    /**
     * @brief Starts the Gateways by setting the gateway configurations
     * and activating the configured gateway
     *
     * This should only be called on containers in state 'READY'
     *
     * @return ReturnCode::SUCCESS If configuration and activation was successful
     * @return ReturnCode::FAILURE If configuration or activation encountered
     *         a non-fatal error
     *
     * @throws GatewayError If configuration or activation of any gateway
     *                      encountered a fatal error.
     * @throws InvalidOperationError If called when state is not 'READY'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    virtual bool startGateways(const GatewayConfiguration &configs) = 0;

    /**
     * @brief Create a job that can run a command in a container
     *
     * This should only be called on containers in state 'READY'
     *
     * @throws InvalidOperationError If called when state is not 'READY'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    virtual std::shared_ptr<CommandJob> createCommandJob(const std::string &command) = 0;

    /**
     * @brief Shot down the container with an explicit timeout
     *
     * Same as the shutdown() method, but the value passed will override any
     * timeout value in the main configuration.
     *
     * @param timeout Seconds to use for timeout
     */
    virtual void shutdown(unsigned int timeout) = 0;

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
    virtual void suspend() = 0;

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
    virtual void resume() = 0;

    /**
     * Should only be called on containers in state 'READY'
     *
     * @throws InvalidOperationError If called when state is not 'READY'
     * @throws InvalidContainerError If the container is in state 'INVALID'
     */
    virtual bool bindMount(const std::string &pathOnHost,
                           const std::string &pathInContainer,
                           bool readonly = true) = 0;

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
    virtual bool previouslyConfigured() = 0;
};

} //namespace
