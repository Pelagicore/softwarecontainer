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

/**
 * @file softwarecontaineragent.h
 * @brief Contains the softwarecontainer::SoftwareContainerAgent class
 */


#include <glib-unix.h>
#include <glibmm.h>

#include <ivi-profiling.h>

#include "filetoolkitwithundo.h"
#include "softwarecontainer.h"
#include "softwarecontainer-common.h"
#include "softwarecontainererror.h"

#include "capability/filteredconfigstore.h"
#include "capability/defaultconfigstore.h"
#include "config/config.h"
#include "containeroptions/containeroptionparser.h"
#include "softwarecontainerfactory.h"

#include <jsonparser.h>
#include "commandjob.h"
#include <queue>

namespace softwarecontainer {

static constexpr ContainerID INVALID_CONTAINER_ID = -1;

/**
 * @brief An error occured in SoftwareContainerAgent
 *
 * This exception should be used if an internal error occurs in the
 * SoftwareContainerAgent.
 *
 */
class SoftwareContainerAgentError : public SoftwareContainerError
{
public:
    SoftwareContainerAgentError():
        m_message("SoftwareContainer error")
    {
    }

    SoftwareContainerAgentError(const std::string &message):
        m_message(message)
    {
    }

    virtual const char *what() const throw()
    {
        return m_message.c_str();
    }

protected:
    std::string m_message;
};


class SoftwareContainerAgent
{
    LOG_DECLARE_CLASS_CONTEXT("SCA", "SoftwareContainerAgent");
    typedef std::shared_ptr<ContainerAgentInterface> SoftwareContainerPtr;

public:
    /**
     * @brief creates a new agent object and runs some initialization
     *
     * This will check if the workspace is sound
     *
     * The config is used to set various values in the agent, or for the agent
     * to pass along to objects it creates.
     *
     * @param mainLoopContext A Glib::MainContext
     * @param config A general SoftwareContainer config
     *
     * @throws SoftwareContainerAgentError if initialization of the agent fails because of
     * the Workspace initialization
     * @throws ConfigStoreError if initialization of ConfigStore fails
     */
    SoftwareContainerAgent(Glib::RefPtr<Glib::MainContext> mainLoopContext,
                           std::shared_ptr<Config> config,
                           std::shared_ptr<SoftwareContainerFactory> factory);

    ~SoftwareContainerAgent();

    /**
     * @brief get a list of all containers
     *
     * @return a vector of container IDs
     */
    std::vector<ContainerID> listContainers();

    /**
     * @brief List all capabilities that the user can set.
     *
     * Capabilities are a mapping from a string to a set of gateway configurations. This method
     * returns all capability names that can be used through setCapabilities.
     *
     * @return a list of capabilities
     * @throws SoftwareContainerError if not possible to create.
     */
    std::vector<std::string> listCapabilities();

    /**
     * @brief delete container by ID
     * @throws SoftwareContainerError on failure.
     */
    void deleteContainer(ContainerID containerID);

    /**
     * @brief Fetches a pointer to a SoftwareContainer matching an ID.
     *
     * @param containerID the ID for the container
     * @return Pointer to the matched container if there is such a container.
     * @throws SoftwareContainerError on failure.
     */
    SoftwareContainerPtr getContainer(ContainerID containerID);

    /**
     * @brief Create a new container
     *
     * @param config container-wide configuration string
     * @param a reference to ContainerID
     *
     * @return ContainerID for the newly created container
     * @throws SoftwareContainerError if not possible to create.
     */
    ContainerID createContainer(const std::string &config);

    /**
     * @brief Launch the given command in a the given container
     *
     * @param containerID the id for the container
     * @param cmdLine the command to run
     * @param workingDirectory the working directory to use when running
     * @param outputFile where to log any output
     * @param env any environment variables to pass to the command
     * @param listener a function that runs when the process exits
     * @return process id of the given command
     */
    pid_t execute(ContainerID containerID,
                 const std::string &cmdLine,
                 const std::string &workingDirectory,
                 const std::string &outputFile,
                 const EnvironmentVariables &env,
                 std::function<void (pid_t, int)> listener);

    /**
     * @brief shuts down a container
     *
     * @param containerID the container to shut down
     * @throws SoftwareContainerError on failure.
     */
    void shutdownContainer(ContainerID containerID);

    /**
     * @brief suspends execution of a container
     *
     * @param containerID the container to suspend
     * @throws SoftwareContainerError on failure.
     */
    void suspendContainer(ContainerID containerID);

    /**
     * @brief resumes execution of a container
     *
     * @param containerID the container to resume
     * @throws SoftwareContainerError on failure.
     */
    void resumeContainer(ContainerID containerID);

    /**
     * @brief Bind mount a folder into the container
     *
     * @param containerID the container to bind into
     * @param pathInHost path to the folder in the host system
     * @param subPathInContainer path (relative to a fixed start point) to use in the container
     * @param readOnly whether or not to mount read-only
     * @param reference to the full path of the mounted folder in the container
     *
     * @throws SoftwareContainerError on failure.
     */
    void bindMount(const ContainerID containerID,
                   const std::string &pathInHost,
                   const std::string &pathInContainer,
                   bool readOnly);

    /**
     * @brief Set capabilities for the container
     *
     * Capabilities are a mapping from a string to a set of gateway configurations. Capabilities
     * are translated into gateway configurations and then sent the same way as with
     * startGateways
     *
     * @param containerID the container to use
     * @param capabilities the capabilities
     *
     * @throws SoftwareContainerError on failure.
    */
    void setCapabilities(const ContainerID &containerID,
                         const std::vector<std::string> &capabilities);

private:
    /**
     * @brief Update gateway configurations for the container
     *
     * @param containerID the container to use
     * @param configs GatewayConfiguration objects
     *
     * @return true on success, false otherwise
     */
    bool updateGatewayConfigs(const ContainerID &containerID,
                              const GatewayConfiguration &configs);

    /**
     * @brief This method cleans unused old containers before agent starts up
     */
    void removeOldContainers(void);


    // Find a job given a pid
    std::shared_ptr<CommandJob> getJob(pid_t pid);
    // Check if a given containerID is valid
    void assertContainerExists(ContainerID containerID);
    // Return a suitable container id
    ContainerID findSuitableId();

    // List of containers in use
    std::map<ContainerID, SoftwareContainerPtr> m_containers;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    SignalConnectionsHandler m_connections;
    std::vector<ContainerID> m_containerIdPool;

    std::shared_ptr<FilteredConfigStore> m_filteredConfigStore;
    std::shared_ptr<DefaultConfigStore>  m_defaultConfigStore;

    std::shared_ptr<Config> m_config;

    /*
     * Holds all configs to use for each SoftwareContainer instance,
     * both the static configs from Config, as well as dynamic values
     * set by the client when creating a container.
     *
     * Each SoftwareContainer instance needs its own copy of these configs
     * so this reference should be used to create a copy from, which should
     * be passed to SoftwareContainer as a unique_ptr.
     */
    SoftwareContainerConfig m_containerConfig;

    /*
     * Responsible for parsing any dynamic values given when creating a container
     */
    ContainerOptionParser m_optionParser;

    /*
     * Responsible to provide connection to containers.
     */
    std::shared_ptr<SoftwareContainerFactory> m_factory;
};

} // namespace softwarecontainer
