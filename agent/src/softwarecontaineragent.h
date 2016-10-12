
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

#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>
#include <getopt.h>

#include <ivi-profiling.h>

#include "SoftwareContainerAgent_dbuscpp_adaptor.h"
#include "softwarecontainer.h"
#include "softwarecontainer-common.h"

#include <jsonparser.h>
#include "commandjob.h"



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
    SoftwareContainerAgent(Glib::RefPtr<Glib::MainContext> mainLoopContext
                     , int preloadCount
                     , bool shutdownContainers
                     , int shutdownTimeout);

    ~SoftwareContainerAgent();

    /**
     * Preload additional containers if needed
     */
    void triggerPreload();

    void deleteContainer(ContainerID containerID);

    /**
     * Check whether the given containerID is valid and return a reference to the actual container
     */
    bool checkContainer(ContainerID containerID, SoftwareContainer * &container);

    ReturnCode readConfigElement(const json_t *element);

    /**
     * Parse config needed for starting up the container in a correct manner.
     */
    bool parseConfig(const std::string &config);

    /**
     * Create a new container
     */
    ContainerID createContainer(const std::string &prefix, const std::string &config);

    bool checkJob(pid_t pid, CommandJob * &result);

    void writeToStdIn(pid_t pid, const std::vector<uint8_t> &bytes);

    /**
     * Launch the given command in a the given container
     */
    pid_t launchCommand(ContainerID containerID, uid_t userID, const std::string &cmdLine,
                        const std::string &workingDirectory, const std::string &outputFile,
                        const EnvironmentVariables &env, std::function<void (pid_t, int)> listener);

    void setContainerName(ContainerID containerID, const std::string &name);

    void shutdownContainer(ContainerID containerID);

    void shutdownContainer(ContainerID containerID, unsigned int timeout);

    std::string bindMountFolderInContainer(const uint32_t containerID, const std::string &pathInHost,
                const std::string &subPathInContainer, bool readOnly);

    void setGatewayConfigs(const uint32_t &containerID, const std::map<std::string, std::string> &configs);

    bool setCapabilities(const uint32_t &containerID, const std::vector<std::string> &capabilities);

    std::shared_ptr<Workspace> getWorkspace();

private:
    std::shared_ptr<Workspace> m_softwarecontainerWorkspace;
    std::map<ContainerID, SoftwareContainerPtr> m_containers;
    std::vector<SoftwareContainerPtr> m_preloadedContainers;
    std::vector<CommandJob *> m_jobs;
    Glib::RefPtr<Glib::MainContext> m_mainLoopContext;
    size_t m_preloadCount;
    SignalConnectionsHandler m_connections;
    bool m_shutdownContainers = true;
    std::vector<ContainerID> m_containerIdPool;
};
}
