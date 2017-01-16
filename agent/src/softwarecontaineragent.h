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


#include "softwarecontainer.h"
#include "softwarecontainer-common.h"
#include "capability/filteredconfigstore.h"
#include "capability/defaultconfigstore.h"
#include "config/config.h"

#include <jsonparser.h>
#include "commandjob.h"
#include <queue>

/**
 * @class softwarecontainer::SoftwareContainerAgent
 * @brief A wrapper class
 */
namespace softwarecontainer {

static constexpr ContainerID INVALID_CONTAINER_ID = -1;

class SoftwareContainerAgent
{
    LOG_DECLARE_CLASS_CONTEXT("SCA", "SoftwareContainerAgent");
    typedef std::shared_ptr<SoftwareContainer> SoftwareContainerPtr;

public:
    /**
     * @brief creates a new agent object and runs some initialization
     *
     * This will check if the workspace is sound, and also call triggerPreload()
     * The config is used to set various values in the agent, or for the agent
     * to pass along to objects it creates.
     *
     * @param mainLoopContext A Glib::MainContext
     * @param config A general SoftwareContainer config
     *
     * @throws ReturnCode::FAILURE if initialization of the agent fails
     */
    SoftwareContainerAgent(Glib::RefPtr<Glib::MainContext> mainLoopContext, std::shared_ptr<Config> config);

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
     * @brief Tries to read a config in json format for a container
     *
     * One can pass options to the agent when creating containers, options that
     * are not gateway specific. These options will be read by this function
     *
     * @param element a config snippet in json format
     * @throws SoftwareContainerError on failure.
     */
    void readConfigElement(const json_t *element);

    /**
     * @brief Parse config needed for starting up the container in a correct manner.
     *
     * This reads a config string into json format, makes sure it is formatted
     * correctly, and then calls readConfigElement for its elements.
     *
     * @param config a configuration string
     */
    void parseConfig(const std::string &config);

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
     * @return process id which belongs to given command
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

    /**
     * @brief get a pointer to the workspace used
     * TODO: This is only used for testing
     *
     * @return a shared pointer to the current workspace
     */
    std::shared_ptr<Workspace> getWorkspace();

private:
    // Get a preloaded container if possible otherwise make a new container.
    std::pair<ContainerID, SoftwareContainerPtr> getContainerPair();

    // Helper for creating software container instances
    SoftwareContainerPtr makeSoftwareContainer(const ContainerID containerID);

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

    // Pre-loads container until the we have as many as configured
    bool triggerPreload();
    // Find a job given a pid
    std::shared_ptr<CommandJob> getJob(pid_t pid);
    // Check if a given containerID is valid
    void assertContainerExists(ContainerID containerID);
    // Return a suitable container id
    ContainerID findSuitableId();

    std::shared_ptr<Workspace> m_softwarecontainerWorkspace;

    // List of containers in use
    std::map<ContainerID, SoftwareContainerPtr> m_containers;

    // Queue of pre-loaded containers
    // Push and pop are the only operations that is applied to this collection
    // That is why it is a std::queue and not a std::map
    std::queue<std::pair<ContainerID, SoftwareContainerPtr>> m_preloadedContainers;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    size_t m_preloadCount;
    SignalConnectionsHandler m_connections;
    std::vector<ContainerID> m_containerIdPool;

    std::shared_ptr<FilteredConfigStore> m_filteredConfigStore;
    std::shared_ptr<DefaultConfigStore>  m_defaultConfigStore;

    std::shared_ptr<Config> m_config;
};

} // namespace softwarecontainer
