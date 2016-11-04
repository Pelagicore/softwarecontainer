
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

/**
 * @file softwarecontaineragent.h
 * @brief Contains the softwarecontainer::SoftwareContainerAgent class
 */


#include <glib-unix.h>
#include <glibmm.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    #include <dbus-c++/dbus.h>
    #include <dbus-c++/glib-integration.h>
#pragma GCC diagnostic pop

#include <getopt.h>

#include <ivi-profiling.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    #include "SoftwareContainerAgent_dbuscpp_adaptor.h"
#pragma GCC diagnostic pop

#include "softwarecontainer.h"
#include "softwarecontainer-common.h"

#include <jsonparser.h>
#include "commandjob.h"
#include <queue>


/**
 * @class softwarecontainer::SoftwareContainerAgent 
 * @brief A wrapper class
 * 
 */
namespace softwarecontainer {

class SoftwareContainerAgent : protected softwarecontainer::JSONParser
{
    LOG_DECLARE_CLASS_CONTEXT("SCA", "SoftwareContainerAgent");
    typedef std::unique_ptr<SoftwareContainer> SoftwareContainerPtr;

public:
    /*
     * @brief creates a new agent object and runs some initialization
     *
     * This will check if the workspace is sound, and also call triggerPreload()
     *
     * @throws ReturnCode::FAILURE if initialization of the agent fails
     */
    SoftwareContainerAgent(Glib::RefPtr<Glib::MainContext> mainLoopContext
            , int preloadCount
            , bool shutdownContainers
            , int shutdownTimeout);

    ~SoftwareContainerAgent();

    /**
     * @brief delete container by ID
     */
    void deleteContainer(ContainerID containerID);

    /**
     * @brief Check whether the given container is valid and return a reference to the actual container
     *
     * @param containerID the ID for the container
     * @param container a pointer to a container object (out parameter)
     */
    bool checkContainer(ContainerID containerID, SoftwareContainer * &container);

    /**
     * @brief Tries to read a config in json format for a container
     *
     * One can pass options to the agent when creating containers, options that
     * are not gateway specific. These options will be read by this function
     *
     * @param element a config snippet in json format
     * @return ReturnCode::SUCCESS
     */
    ReturnCode readConfigElement(const json_t *element);

    /**
     * @brief Parse config needed for starting up the container in a correct manner.
     *
     * This reads a config string into json format, makes sure it is formatted
     * correctly, and then calls readConfigElement for its elements.
     *
     * @param config a configuration string
     */
    bool parseConfig(const std::string &config);

    /**
     * @brief Create a new container
     *
     * @param config container-wide configuration string
     *
     * @return If the container is successfully created a positive ContainerID
     *         representing the newly created container will be returned.
     *         Otherwise, -1 will be returned.
     *
     */
    ContainerID createContainer(const std::string &config);


    /**
     * @brief writes to stdin of a process inside a container
     *
     * @param pid the pid of the process (must be created through launchCommand)
     * @param bytes the bytes to write to stdin
     */
    void writeToStdIn(pid_t pid, const std::vector<uint8_t> &bytes);

    /**
     * @brief Launch the given command in a the given container
     *
     * @param containerID the id for the container
     * @param userID the userID to use when running the command inside the container
     * @param cmdLine the command to run
     * @param workingDirectory the working directory to use when running
     * @param outputFile where to log any output
     * @param env any environment variables to pass to the command
     * @param listener a function that runs when the process exits
     */
    pid_t launchCommand(ContainerID containerID, uid_t userID, const std::string &cmdLine,
            const std::string &workingDirectory, const std::string &outputFile,
            const EnvironmentVariables &env, std::function<void (pid_t, int)> listener);

    /**
     * @brief sets the container name
     *
     * @param containerID the id for the container
     * @param name the name to set
     */
    void setContainerName(ContainerID containerID, const std::string &name);

    /**
     * @brief shuts down a container
     *
     * @param containerID the container to shut down
     */
    void shutdownContainer(ContainerID containerID);

    /**
     * @brief shuts down a container with a custom timeout
     *
     * @param containerID the container to shut down
     * @param timeout timeout in seconds to wait before forcing
     */
    void shutdownContainer(ContainerID containerID, unsigned int timeout);

    /**
     * @brief Bind mount a folder into the container
     *
     * @param containerID the container to bind into
     * @param pathInHost path to the folder in the host system
     * @param subPathInContainer path (relative to a fixed start point) to use in the container
     * @param readOnly whether or not to mount read-only
     *
     * @return the full path of the mounted folder in the container
     */
    std::string bindMountFolderInContainer(const ContainerID containerID, const std::string &pathInHost,
            const std::string &subPathInContainer, bool readOnly);

    /**
     * @brief Set configuration for the container gateways
     *
     * @param containerID the container to use
     * @param configs a mapping from gateway-ids to configuration strings to send to the container
     */
    void setGatewayConfigs(const ContainerID &containerID, const std::map<std::string, std::string> &configs);

    /**
     * @brief Set capabilities for the container
     *
     * Capabilities are a mapping from a string to a set of gateway configurations. Capabilities
     * are translated into gateway configurations and then sent the same way as with
     * setGatewayConfigs
     *
     * @param containerID the container to use
     * @param capabilities the capabilities
     *
     * @return true on success, false otherwise
     */
    bool setCapabilities(const ContainerID &containerID, const std::vector<std::string> &capabilities);

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
    SoftwareContainerPtr makeSoftwareContainer(const ContainerID &containerID);

    // Pre-loads container until the we have as many as configured
    bool triggerPreload();
    // Find a job given a pid
    bool checkJob(pid_t pid, CommandJob * &result);
    // Check if a given containerID is valid
    inline bool isIdValid (ContainerID containerID);
    // Return a suitable container id
    ContainerID findSuitableId();

    std::shared_ptr<Workspace> m_softwarecontainerWorkspace;

    // List of containers in use
    std::map<ContainerID, SoftwareContainerPtr> m_containers;

    // Queue of pre-loaded containers
    // Push and pop are the only operations that is applied to this collection
    // That is why it is a std::queue and not a std::map
    std::queue<std::pair<ContainerID, SoftwareContainerPtr>> m_preloadedContainers;
    // List of running jobs
    std::vector<CommandJob *> m_jobs;

    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    size_t m_preloadCount;
    SignalConnectionsHandler m_connections;
    bool m_shutdownContainers = true;
    std::vector<ContainerID> m_containerIdPool;
};
}
