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


#include <vector>
#include <fstream>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>

// rlimit stuff
#include <sys/time.h>
#include <sys/resource.h>

#include <lxc/lxccontainer.h>
#include <lxc/version.h>

#include <pwd.h>
#include <grp.h>

#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <mntent.h>

#include "container.h"
#include "filecleanuphandler.h"

#include <libgen.h>

#include "softwarecontainererror.h"

namespace softwarecontainer {

static constexpr const char *LXC_CONTAINERS_ROOT_CONFIG_ITEM = "lxc.lxcpath";

std::vector<const char *> Container::s_LXCContainerStates;
const char *Container::s_LXCRoot;

void Container::init_lxc()
{
    static bool bInitialized = false;
    if (!bInitialized) {
        int stateCount = lxc_get_wait_states(nullptr);
        s_LXCContainerStates.resize(stateCount);
        lxc_get_wait_states(s_LXCContainerStates.data());
        if ((int)LXCContainerState::ELEMENT_COUNT != s_LXCContainerStates.size()) {
            log_error() << "Internal SC/LXC state mis-match, fatal error";
        } else {
            bInitialized = true;
            s_LXCRoot = lxc_get_global_config_item(LXC_CONTAINERS_ROOT_CONFIG_ITEM);
        }
    }
}

Container::Container(const std::string id,
                     const std::string &configFile,
                     const std::string &containerRoot,
                     bool writeBufferEnabled,
                     int shutdownTimeout) :
    m_configFile(configFile),
    m_id(id),
    m_containerRoot(containerRoot),
    m_writeBufferEnabled(writeBufferEnabled),
    m_shutdownTimeout(shutdownTimeout)
{
    init_lxc();
    log_debug() << "Container constructed with " << id;
}

Container::~Container()
{
    if (m_container != nullptr) {
        // These will check the current state

        if (m_state >= ContainerState::STARTED) {
            shutdown();
        }

        if (m_state >= ContainerState::CREATED) {
            destroy();
        }

        // Any existing container
        lxc_container_put(m_container);
        m_container = nullptr;
    }
}

bool Container::initialize()
{
    if (m_state < ContainerState::PREPARED) {
        std::string gatewayDir = gatewaysDir();
        std::unique_ptr<CreateDir> createDirInstance = std::unique_ptr<CreateDir>(new CreateDir());
        if (!createDirInstance->createDirectory(gatewayDir)) {
            log_error() << "Could not create gateway directory "
                        << gatewayDir << ": " << strerror(errno);
            return false;
        }

        if (!createSharedMountPoint(gatewayDir)) {
            log_error() << "Could not create shared mount point for dir: " << gatewayDir;
            return false;
        }
        m_createDirList.push_back(std::move(createDirInstance));
        m_state = ContainerState::PREPARED;
    }
    return true;
}

std::string Container::toString()
{
    std::stringstream ss;
    ss << "LXC " << id() << " ";
    if (m_container != nullptr) {
        ss << "id: " << id()
           << " / state:" << m_container->state(m_container)
           << " / initPID:" << m_container->init_pid(m_container)
           << " / LastError: " << m_container->error_string
           << " / To connect to this container : lxc-attach -n " << id();
    }

    return ss.str();
}

bool Container::create()
{
    if (m_state >= ContainerState::CREATED) {
        log_warning() << "Container already created";
        return false;
    }

    log_debug() << "Creating container " << toString();

    const char *containerID = id();
    if (strlen(containerID) == 0) {
        log_error() << "ContainerID cannot be empty";
        return false;
    }

    setenv("GATEWAY_DIR", gatewaysDir().c_str(), true);
    log_debug() << "GATEWAY_DIR : " << Glib::getenv("GATEWAY_DIR");

    auto configFile = m_configFile.c_str();
    log_debug() << "Config file : " << configFile;
    log_debug() << "Template : " << LXCTEMPLATE;
    log_debug() << "creating container with ID : " << containerID;

    // Creating a new LXC pointer
    // After this point all failures should do rollback
    m_container = lxc_container_new(containerID, nullptr);
    if (!m_container) {
        log_error() << "Error creating a new container";
        return rollbackCreate();
    }

    if (m_container->is_defined(m_container)) {
        log_error() << "ContainerID '" << containerID << "' is already in use.";
        return rollbackCreate();
    }
    log_debug() << "Successfully created container struct";

    if (!m_container->load_config(m_container, configFile)) {
        log_error() << "Error loading container config";
        return rollbackCreate();

    }
    log_debug() << "Successfully loaded container config";

    // File system stuff
    m_rootFSPath = buildPath(s_LXCRoot, containerID, "rootfs");

    if (m_writeBufferEnabled) {
        const std::string rootFSPathLower = m_containerRoot + "/rootfs-lower";
        const std::string rootFSPathUpper = m_containerRoot + "/rootfs-upper";
        const std::string rootFSPathWork  = m_containerRoot + "/rootfs-work";

        overlayMount(rootFSPathLower, rootFSPathUpper, rootFSPathWork, m_rootFSPath);
        log_debug() << "Write buffer enabled, lowerdir=" << rootFSPathLower
                    << ", upperdir=" << rootFSPathUpper
                    << ", workdir=" << rootFSPathWork
                    << ", dst=" << m_rootFSPath;
    } else {
        log_debug() << "WriteBuffer disabled, dst=" << m_rootFSPath;
    }

    // Set the LXC template
    int flags = 0;
    std::vector<char *> argv;
    if (!m_container->create(m_container, LXCTEMPLATE, nullptr, nullptr, flags, &argv[0])) {
        log_error() << "Error creating container";
        m_rootFSPath.assign("");
        return rollbackCreate();
    }

    // Everything went fine, set state and return successfully.
    m_state = ContainerState::CREATED;
    log_debug() << "Container created. RootFS: " << m_rootFSPath;

    return true;
}

bool Container::rollbackCreate() {
    if (m_container) {
        lxc_container_put(m_container);
        m_container = nullptr;
    }
    return false;
}

bool Container::ensureContainerRunning()
{
    if (ContainerState::FROZEN == m_state) {
        log_error() << "Container is frozen, does not run";
        return false;
    }

    if (m_state < ContainerState::STARTED) {
        log_error() << "Container is not in state STARTED, state is " << ((int)m_state);
        log_error() << logging::getStackTrace();
        return false;
    }

    if (!m_container->is_running(m_container)) {
        return waitForState(LXCContainerState::RUNNING);
    }

    return true;
}

bool Container::waitForState(LXCContainerState state, int timeout)
{
    const char* currentState = m_container->state(m_container);
    if (strcmp(currentState, toString(state))) {
        log_debug() << "Waiting for container to change from " << currentState
                    << " to state : " << toString(state);
        bool expired = m_container->wait(m_container, toString(state), timeout);
        if (expired) {
            log_error() << "Container did not reach" << toString(state) << " in time";
            return false;
        }
    }
    return true;
}

bool Container::start(pid_t *pid)
{
    if (m_state < ContainerState::CREATED) {
        log_warning() << "Trying to start container that isn't created. Please create the container first";
        return false;
    }

    if (pid == nullptr) {
        log_error() << "Supplied pid argument is nullptr";
        return false;
    }

    // For some reason the LXC start function does not work out
    log_debug() << "Starting container";

    char commandEnv[] = "env";
    char commandSleep[] = "/bin/sleep";
    char commandSleepTime[] = "100000000";
    char* const args[] = { commandEnv, commandSleep, commandSleepTime, nullptr };

    if (!m_container->start(m_container, false, args)) {
        log_error() << "Error starting container";
        return false;
    }

    log_debug() << "Container started: " << toString();
    *pid = m_container->init_pid(m_container);
    m_state = ContainerState::STARTED;

    if (!ensureContainerRunning()) {
        log_error() << "Container started but is not running";
        return false;
    }

    log_info() << "To connect to this container : lxc-attach -n " << id();
    return true;
}

int Container::unlimitCoreDump()
{
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) != 0) {
        return errno;
    }

    // Set this to the maximum allowed value (ulimit -c unlimited if root)
    rlim.rlim_cur = rlim.rlim_max;
    if (setrlimit(RLIMIT_CORE, &rlim) != 0) {
        return errno;
    }

    return 0;
}

bool Container::setCgroupItem(std::string subsys, std::string value)
{
    return m_container->set_cgroup_item(m_container, subsys.c_str(), value.c_str());
}

int Container::executeInContainerEntryFunction(void *param)
{
    int canCoreDump = unlimitCoreDump();
    if (canCoreDump != 0) {
        return canCoreDump;
    }

    ExecFunction *function = (ExecFunction *) param;
    return (*function)();
}

bool Container::executeSync(ExecFunction function,
                            pid_t *pid,
                            const EnvironmentVariables &variables,
                            int stdin_var,
                            int stdout_var,
                            int stderr_var)
{
    bool execResult = execute(function, pid, variables, stdin_var, stdout_var, stderr_var);

    // Make sure the container execution finishes and was successful
    int status = waitForProcessTermination(*pid);
    return execResult && (0 == status);
}

bool Container::execute(ExecFunction function,
                        pid_t *pid,
                        const EnvironmentVariables &variables,
                        int stdin,
                        int stdout,
                        int stderr)
{
    if (pid == nullptr) {
        log_error() << "Supplied pid argument is nullptr";
        return false;
    }

    if (!ensureContainerRunning()) {
        log_error() << "Container is not running or in bad state, can't execute";
        return false;
    }

    lxc_attach_options_t options = LXC_ATTACH_OPTIONS_DEFAULT;
    options.stdin_fd = stdin;
    options.stdout_fd = stdout;
    options.stderr_fd = stderr;

    options.uid = ROOT_UID;
    options.gid = ROOT_UID;

    // List of vars to use when executing the function
    EnvironmentVariables actualVariables = variables;

    // Add the variables set by gateways, variables passed with the 'variables' argument
    // will take precedence over previously set variables.
    for (auto &var : m_gatewayEnvironmentVariables) {
        if (variables.count(var.first) != 0) {
            if (m_gatewayEnvironmentVariables.at(var.first) != variables.at(var.first)) {
                // Inform user that GW config will be overridden, it might be unintentionally done
                log_info() << "Variable \"" << var.first
                           << "\" set by gateway will be overwritten with the value: \""
                           << variables.at(var.first) << "\"";
            }
            actualVariables[var.first] = variables.at(var.first);
        } else {
            // The variable was not set again, just keep the original value set by GW
            actualVariables[var.first] = var.second;
        }
    }

    log_debug() << "Starting function in container " << toString();

    // prepare array of env variable strings to be set when launching the process in the container
    std::vector<std::string> strings;
    for (auto &var : actualVariables) {
        strings.push_back(var.first + "=" + var.second);
    }

    const size_t stringCount = strings.size() + 1;
    const char **envVariablesArray = new const char* [stringCount];
    for (size_t i = 0; i < strings.size(); i++) {
        envVariablesArray[i] = strings[i].c_str();
        log_debug() << "Passing env variable: " << strings[i];
    }
    // Null terminate
    envVariablesArray[strings.size()] = nullptr;
    options.extra_env_vars = (char **)envVariablesArray;

    // Do the actual attach call to the container
    int attach_res = m_container->attach(m_container,
                                         &Container::executeInContainerEntryFunction,
                                         &function,
                                         &options,
                                         pid);

    delete[] envVariablesArray;
    if (attach_res == 0) {
        log_info() << " Attached PID: " << *pid;
        return true;
    } else  {
        log_error() << "Attach call to LXC container failed: " << std::string(strerror(errno));
        return false;
    }
}

bool Container::execute(const std::string &commandLine, pid_t *pid,
                        const EnvironmentVariables &variables,
                        const std::string &workingDirectory,
                        int stdin, int stdout, int stderr)
{
    if (!ensureContainerRunning()) {
        log_error() << "Container is not running or in bad state, can't attach";
        return false;
    }

    if (pid == nullptr) {
        log_error() << "Supplied pid argument is nullptr";
        return false;
    }

    log_debug() << "Attach " << commandLine ;

    std::vector<std::string> executeCommandVec = Glib::shell_parse_argv(commandLine);
    std::vector<char *> args;

    for (size_t i = 0; i < executeCommandVec.size(); i++) {
        executeCommandVec[i].c_str(); // ensure the string is null-terminated. not sure thas is required.
        auto s = &executeCommandVec[i][0];
        args.push_back(s);
    }

    // We need a null terminated array
    args.push_back(nullptr);

    bool result = execute([&] () {
                if (workingDirectory.length() != 0) {
                    auto ret = chdir(workingDirectory.c_str());
                    if (ret != 0) {
                        log_error() << "Error when changing current directory : "
                                    << strerror(errno);
                    }
                }
                execvp(args[0], args.data());
                return 1;
            }, pid, variables, stdin, stdout, stderr);
    if (!result) {
        log_error() << "Could not execute in container";
        return false;
    }

    return true;
}

bool Container::stop()
{
    bool ret = true;
    if (m_state >= ContainerState::STARTED) {
        log_debug() << "Stopping the container";
        if (m_container->stop(m_container)) {
            log_debug() << "Container stopped, waiting for stop state";
            waitForState(LXCContainerState::STOPPED);
        } else {
            log_error() << "Unable to stop container";
            ret = false;
        }
    } else {
        log_error() << "Can't stop container that has not been started";
        ret = false;
    }

    return ret;
}

bool Container::shutdown()
{
    return shutdown(m_shutdownTimeout);
}

bool Container::shutdown(unsigned int timeout)
{
    if (m_state < ContainerState::STARTED) {
        log_error() << "Trying to shutdown container that has not been started. Aborting";
        return false;
    }

    log_debug() << "Shutting down container " << toString() << " pid: "
                << m_container->init_pid(m_container);

    if (m_container->init_pid(m_container) != INVALID_PID) {
        kill(m_container->init_pid(m_container), SIGTERM);
    }

    // Shutdown with timeout
    bool success = m_container->shutdown(m_container, timeout);
    if (!success) {
        log_warning() << "Failed to cleanly shutdown container, forcing stop" << toString();
        if(!stop()) {
            log_error() << "Failed to force stop the container" << toString();
            return false;
        }
    }

    m_state = ContainerState::CREATED;
    return true;
}

bool Container::destroy()
{
    return destroy(m_shutdownTimeout);
}

bool Container::destroy(unsigned int timeout)
{
    if (m_state < ContainerState::CREATED) {
        log_error() << "Trying to destroy container that has not been created. Aborting destroy";
        return false;
    }

    if (m_state >= ContainerState::STARTED) {
        if (!shutdown(timeout)) {
            log_error() << "Could not shutdown container. Aborting destroy";
            return false;
        }
    }


    // The container can not be destroyed unless the rootfs is unmounted
    if (m_writeBufferEnabled)
    {
        log_debug() << "Unmounting the overlay rootfs";
        if(-1 == umount(m_rootFSPath.c_str())) {
            log_error() << "Unmounting the overlay rootfs failed: " << strerror(errno);
            return false;
        }
    }

    // Destroy it!
    bool success = m_container->destroy(m_container);
    if (!success) {
        log_error() << "Failed to destroy the container " << toString();
        return false;
    }

    m_state = ContainerState::DESTROYED;
    return true;
}

bool Container::bindMountInContainer(const std::string &pathInHost,
                                     const std::string &pathInContainer,
                                     bool readOnly)
{
    if (!existsInFileSystem(pathInHost)) {
        log_error() << "Path on host does not exist: " << pathInHost;
        return false;
    }

    // Check that there is nothing already mounted on the target path
    pid_t pid = INVALID_PID;
    bool checkIfAlreadyMountpoint = executeSync([pathInContainer] () {
        FILE *mountsfile = setmntent("/proc/mounts", "r");
        struct mntent *mounts;
        while ((mounts = getmntent(mountsfile)) != nullptr) {
            std::string mountDir(mounts->mnt_dir);
            if (0 == mountDir.compare(pathInContainer)) {
                return -1;
            }
        }
        return 0;
    }, &pid);

    if (!checkIfAlreadyMountpoint) {
        log_error() << pathInContainer << " is already mounted to.";
        return false;
    }

    // Create a file to mount to in gateways
    std::string filePart = baseName(pathInContainer);
    std::string tempPath = buildPath(gatewaysDir(), filePart);
    bool pathIsDirectory = false;
    std::unique_ptr<CreateDir> createDirInstance = std::unique_ptr<CreateDir>(new CreateDir());
    //
    // If the path is a directory, we create the tempPath (which adds a cleanup handler).
    //
    // If it is a file, we touch the file and add a cleanup handler for it.
    //
    if (isDirectory(pathInHost)) {
        pathIsDirectory = true;
        log_debug() << "Path on host (" << pathInHost << ") is directory, mounting as a directory";

        log_debug() << "Creating folder : " << tempPath;
        if (!createDirInstance->createDirectory(tempPath)) {
            log_error() << "Could not create folder " << tempPath;
            return false;
        }
    } else {
        // This goes for sockets, fifos etc as well.
        log_debug() << "Path on host (" << pathInHost << ") is not a directory, "
                    << "mounting assuming it behaves like a file";

        if (!pathInList(tempPath)) {
            if (!touch(tempPath)) {
                log_error() << "Could not create file " << tempPath;
                return false;
            }
        }
    }

    if (!bindMountCore(pathInHost, pathInContainer, tempPath, readOnly)) {
        // bindMountInContainer operation is not successful. Clean mounted tempPath.
        MountCleanUpHandler rollback{tempPath};
        rollback.clean();
        return false;
    }

    // bindMountInContainer succeed. Add cleanup for mounted tempPath
    m_createDirList.push_back(std::move(createDirInstance));
    if (!pathIsDirectory) {
        m_cleanupHandlers.emplace_back(new FileCleanUpHandler(tempPath));
    }

    m_cleanupHandlers.emplace_back(new MountCleanUpHandler(tempPath));
    return true;
}

bool Container::bindMountCore(const std::string &pathInHost,
                              const std::string &pathInContainer,
                              const std::string &tempDirInContainerOnHost,
                              bool readonly)
{
    if (!ensureContainerRunning()) {
        log_error() << "Container is not running or in bad state, can't bind-mount folder";
        return false;
    }

    if (pathInContainer.front() != '/') {
        log_error() << "Provided path '" << pathInContainer << "' is not absolute!";
        return false;
    }

    // Bind mount to /gateways
    if (!bindMount(pathInHost,
                   tempDirInContainerOnHost,
                   m_containerRoot,
                   readonly,
                   m_writeBufferEnabled)) {
        log_error() << "Could not bind mount " << pathInHost << " to " << tempDirInContainerOnHost;
        return false;
    }

    // Paths inside the container
    std::string filePart = baseName(pathInContainer);
    std::string tempDirInContainer = buildPath(gatewaysDirInContainer(), filePart);

    pid_t pid = INVALID_PID;

    // Utility lambda function to create parent paths of a given path
    auto createParentDirectories = [this] (std::string path) {
        std::stack<std::string> paths;
        std::string currentPath = path;
        while(!isDirectory(currentPath)) {
            paths.push(currentPath);
            currentPath = parentPath(currentPath);
        }

        while(!paths.empty()) {
            std::string path = paths.top();
            if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
                return 1;
            }

            paths.pop();
        }
        return 0;
    };

    // Move the mount in the container if the tempdir is not the desired dir
    if (tempDirInContainer.compare(pathInContainer) != 0) {
        //
        // First, optionally create any parent directories to the targets
        // We do this on the host to avoid working too much inside the container
        //
        std::string parentPathInContainer = parentPath(pathInContainer);
        ExecFunction createParent = std::bind(createParentDirectories, parentPathInContainer);

        if (!executeSync(createParent, &pid)) {
            log_error() << "Could not create parent directory " << parentPathInContainer
                        << " in container";
            return false;
        }

        //
        // Then, create the actual directory / file to mount to.
        //
        if (isDirectory(tempDirInContainerOnHost)) {
            ExecFunction createDir = std::bind(createParentDirectories, pathInContainer);

            if (!executeSync(createDir, &pid)) {
                log_error() << "Could not create target directory " << pathInContainer
                            << " in the container";
                return false;
            }
        } else {
            log_debug() << "Touching file in container: " << pathInContainer;
            bool touchResult = executeSync([pathInContainer] () {
                return touch(pathInContainer) ? 0 : 1;
            }, &pid);

            if (!touchResult) {
                log_error() << "Could not touch target file " << pathInContainer
                            << " in the container";
                return false;
            }
        }

        //
        // And move the mount from /gateways to the desired location
        //

        bool mountMoveResult = executeSync([tempDirInContainer, pathInContainer] () {
            unsigned long flags = MS_MOVE;
            int ret = mount(tempDirInContainer.c_str(), pathInContainer.c_str(), nullptr, flags, nullptr);
            if (ret != 0) {
                printf("Error while moving the mount %s to %s: %s\n",
                       tempDirInContainer.c_str(),
                       pathInContainer.c_str(),
                       strerror(errno));
            }
            return ret;
        }, &pid);

        if (!mountMoveResult) {
            log_error() << "Could not move the mount inside the container: "
                        << tempDirInContainer << " to " << pathInContainer;
            return false;
        }
    }

    // Remount read only in the container if applicable
    if (readonly && !remountReadOnlyInContainer(pathInContainer)) {
       log_error() << "Failed to remount read only: " << pathInContainer;
       return false;
    }

    return true;
}

bool Container::remountReadOnlyInContainer(const std::string &path)
{
    pid_t pid = INVALID_PID;

    bool ret = executeSync([path] () {
        unsigned long flags = MS_REMOUNT | MS_RDONLY | MS_BIND;
        return mount(path.c_str(), path.c_str(), "", flags, nullptr);
    }, &pid);

    if (!ret) {
        log_error() << "Could not remount " << path << " read-only in container";
        return false;
    }

    return true;
}

bool Container::mountDevice(const std::string &pathInHost)
{
    if(!ensureContainerRunning()) {
        log_error() << "Container is not running or in bad state, can't mount device: " << pathInHost;
        return false;
    }
    log_debug() << "Mounting device in container : " << pathInHost;
    return m_container->add_device_node(m_container, pathInHost.c_str(), nullptr);
}

bool Container::setEnvironmentVariable(const std::string &var, const std::string &val)
{
    if (m_state < ContainerState::CREATED) {
        log_error() << "Can't set environment variable for non-created container";
        return false;
    }

    log_debug() << "Setting env variable in container " << var << "=" << val;
    m_gatewayEnvironmentVariables[var] = val;

    // We generate a file containing all variables for convenience when connecting to the container in command-line
    logging::StringBuilder s;
    for (auto &var : m_gatewayEnvironmentVariables) {
        s << "export " << var.first << "='" << var.second << "'\n";
    }
    std::string path = buildPath(gatewaysDir(), "env");
    FileToolkitWithUndo::writeToFile(path, s);

    return true;
}

bool Container::suspend()
{
    if (m_state < ContainerState::STARTED) {
        log_error() << "Container is not started yet";
        return false;
    }

    if (m_state == ContainerState::FROZEN) {
        log_error() << "Container is already suspended";
        return false;
    }

    log_debug() << "Suspending container";
    bool retval = m_container->freeze(m_container);

    if (!retval) {
        std::string errorMessage("Could not suspend the container.");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    m_state = ContainerState::FROZEN;
    return true;
}

bool Container::resume()
{
    if (m_state < ContainerState::FROZEN) {
        log_error() << "Container is not suspended";
        return false;
    }

    log_debug() << "Resuming container";
    bool retval = m_container->unfreeze(m_container);

    if (!retval) {
        std::string errorMessage("Could not resume the container.");
        log_error() << errorMessage;
        throw SoftwareContainerError(errorMessage);
    }

    m_state = ContainerState::STARTED;
    return true;
}

const char *Container::id() const
{
    return m_id.c_str();
}

std::string Container::gatewaysDirInContainer() const
{
    return GATEWAYS_PATH;
}

std::string Container::gatewaysDir() const
{
    return buildPath(m_containerRoot, GATEWAYS_PATH);
}

} // namespace softwarecontainer
