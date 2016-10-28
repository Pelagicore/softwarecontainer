
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


#include <vector>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <lxc/lxccontainer.h>
#include <lxc/version.h>

#include <pwd.h>
#include <grp.h>

#include <errno.h>
#include <string.h>

#include "container.h"
#include "filecleanuphandler.h"

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

Container::Container(const std::string id, const std::string &configFile,
        const std::string &containerRoot, bool enableWriteBuffer, int shutdownTimeout) :
    m_configFile(configFile),
    m_id(id),
    m_containerRoot(containerRoot),
    m_enableWriteBuffer(enableWriteBuffer),
    m_shutdownTimeout(shutdownTimeout)
{
    log_debug() << "Container constructed with " + id;
    init_lxc();
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

ReturnCode Container::initialize()
{
    if (m_state < ContainerState::PREPARED) {
        std::string gatewayDir = gatewaysDir();
        if (isError(createDirectory(gatewayDir))) {
            log_error() << "Could not create gateway directory " << gatewayDir << strerror(errno);
            return ReturnCode::FAILURE;
        }

        if (isError(createSharedMountPoint(gatewayDir))) {
            log_error() << "Could not create shared mount point for dir: " << gatewayDir;
            return ReturnCode::FAILURE;
        }

        m_state = ContainerState::PREPARED;
    }
    return ReturnCode::SUCCESS;
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

ReturnCode Container::create()
{
    if (m_state >= ContainerState::CREATED) {
        log_warning() << "Container already created";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Creating container " << toString();
    ReturnCode status = ReturnCode::SUCCESS;

    const char *containerID = id();
    if (strlen(containerID) == 0) {
        log_error() << "ContainerID cannot be empty";
        return ReturnCode::FAILURE;
    }

    setenv("GATEWAY_DIR", gatewaysDir().c_str(), true);
    log_debug() << "GATEWAY_DIR : " << getenv("GATEWAY_DIR");

    auto configFile = m_configFile.c_str();
    log_debug() << "Config file : " << configFile;
    log_debug() << "Template : " << LXCTEMPLATE;
    log_debug() << "creating container with ID : " << containerID;

    m_container = lxc_container_new(containerID, nullptr);
    if (!m_container) {
        log_error() << "Error creating a new container";
        status = ReturnCode::FAILURE;
    } else {
        log_debug() << "Successfully created container struct";
    }

    if (isSuccess(status)) {
        if (!m_container->load_config(m_container, configFile)) {
            log_error() << "Error loading container config";
            status = ReturnCode::FAILURE;
        } else {
            log_debug() << "Successfully loaded container config";
        }
    }

    if (isSuccess(status)) {
        int flags = 0;
        std::vector<char *> argv;

        const std::string rootfspath_dst = StringBuilder() << s_LXCRoot << "/" << containerID << "/rootfs";

        if (m_enableWriteBuffer) {
            createDirectory(rootfspath_dst);
            const std::string rootfspath_lower = rootfspath_dst + "-lower";
            createDirectory(rootfspath_lower);
            const std::string rootfspath_upper = rootfspath_dst + "-upper";
            createDirectory(rootfspath_upper);
            const std::string rootfspath_work = rootfspath_dst + "-work";
            overlayMount(rootfspath_lower, rootfspath_upper, rootfspath_work, rootfspath_dst);
            m_rootFSPath = rootfspath_dst;

            log_debug() << "Write buffer enabled, lower=" << rootfspath_lower
                        << ", upper=" << rootfspath_upper
                        << ", work=" << rootfspath_work
                        << ", dst=" << rootfspath_dst;
        } else {
            m_rootFSPath = rootfspath_dst;
            log_debug() << "WriteBuffer disabled, dst=" << rootfspath_dst;
        }

        if (!m_container->create(m_container, LXCTEMPLATE, nullptr, nullptr, flags, &argv[0])) {
            log_error() << "Error creating container";
            m_rootFSPath.assign("");
            status = ReturnCode::FAILURE;
        } else {
            m_state = ContainerState::CREATED;
            log_debug() << "Container created. RootFS: " << m_rootFSPath;
        }
    }

    if(isError(status)) {
        lxc_container_put(m_container);
        m_container = nullptr;
    }

    return status;
}

ReturnCode Container::ensureContainerRunning()
{
    if (m_state < ContainerState::STARTED) {
        log_error() << "Container is not in state STARTED, state is " << ((int)m_state);
        log_error() << logging::getStackTrace();
        return ReturnCode::FAILURE;
    }

    if (!m_container->is_running(m_container)) {
        return waitForState(LXCContainerState::RUNNING);
    }

    return ReturnCode::SUCCESS;
}

ReturnCode Container::waitForState(LXCContainerState state, int timeout)
{
    const char* currentState = m_container->state(m_container);
    if (strcmp(currentState, toString(state))) {
        log_debug() << "Waiting for container to change from " << currentState
                    << " to state : " << toString(state);
        bool b = m_container->wait(m_container, toString(state), timeout);
        if (b) {
            log_error() << "Container did not reach" << toString(state) << " in time";
            return ReturnCode::FAILURE;
        }
    }
    return ReturnCode::SUCCESS;
}

ReturnCode Container::start(pid_t *pid)
{
    if (m_state < ContainerState::CREATED) {
        log_warning() << "Trying to start container that isn't created. Please create the container first";
        return ReturnCode::FAILURE;
    }

    if (pid == nullptr) {
        log_error() << "Supplied pid argument is nullptr";
        return ReturnCode::FAILURE;
    }

    // For some reason the LXC start function does not work out
    log_debug() << "Starting container";

    char commandEnv[] = "env";
    char commandSleep[] = "/bin/sleep";
    char commandSleepTime[] = "100000000";
    char* const args[] = { commandEnv, commandSleep, commandSleepTime, nullptr};

    if (!m_container->start(m_container, false, args)) {
        log_error() << "Error starting container";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Container started: " << toString();
    *pid = m_container->init_pid(m_container);
    m_state = ContainerState::STARTED;

    if (isError(ensureContainerRunning())) {
        log_error() << "Container started but is not running";
        return ReturnCode::FAILURE;
    }

    log_info() << "To connect to this container : lxc-attach -n " << id();
    return ReturnCode::SUCCESS;
}

int Container::executeInContainerEntryFunction(void *param)
{
    ContainerFunction *function = (ContainerFunction *) param;
    return (*function)();
}

ReturnCode Container::executeInContainer(ContainerFunction function, pid_t *pid, const EnvironmentVariables &variables, uid_t userID,
                                         int stdin, int stdout, int stderr)
{
    if (pid == nullptr) {
        log_error() << "Supplied pid argument is nullptr";
        return ReturnCode::FAILURE;
    }

    if (isError(ensureContainerRunning())) {
        log_error() << "Container is not running or in bad state, can't execute";
        return ReturnCode::FAILURE;
    }

    lxc_attach_options_t options = LXC_ATTACH_OPTIONS_DEFAULT;
    options.stdin_fd = stdin;
    options.stdout_fd = stdout;
    options.stderr_fd = stderr;

    options.uid = userID;
    options.gid = userID;

    EnvironmentVariables actualVariables = variables;

    // Add the variables set by the gateways
    for (auto &var : m_gatewayEnvironmentVariables) {
        if (variables.count(var.first) != 0) {
            if (m_gatewayEnvironmentVariables.at(var.first) != variables.at(var.first)) {
                log_warning() << "Variable set by gateway overriding original variable value: "
                              << var.first << ". values: " << var.second;
            }
        }

        actualVariables[var.first] = var.second;
    }

    // prepare array of env variable strings to be set when launching the process in the container
    std::vector<std::string> strings;
    for (auto &var : actualVariables) {
        strings.push_back(var.first + "=" + var.second);
    }

    const size_t stringCount = strings.size() + 1;
    const char **envVariablesArray = new const char* [stringCount];
    for (size_t i = 0; i < strings.size(); i++) {
        envVariablesArray[i] = strings[i].c_str();
    }
    envVariablesArray[strings.size()] = nullptr;
    options.extra_env_vars = (char * *) envVariablesArray;

    log_debug() << "Starting function in container "
                << toString() << "User:" << userID
                << "" << std::endl << " Env variables : " << strings;

    int attach_res = m_container->attach(m_container,
                                         &Container::executeInContainerEntryFunction,
                                         &function, &options, pid);

    delete envVariablesArray;
    if (attach_res == 0) {
        log_info() << " Attached PID: " << *pid;
        return ReturnCode::SUCCESS;
    } else  {
        log_error() << "Attach call to LXC container failed: " << std::string(strerror(errno));
        return ReturnCode::FAILURE;
    }
}


ReturnCode Container::setCgroupItem(std::string subsys, std::string value)
{
    bool success = m_container->set_cgroup_item(m_container, subsys.c_str(), value.c_str());
    return bool2ReturnCode(success);
}

ReturnCode Container::setUser(uid_t userID)
{
    //log_info() << "Setting env for userID : " << userID;

    struct passwd *pw = getpwuid(userID);
    if (pw != nullptr) {
        std::vector<gid_t> groups;
        int ngroups = 0;

        if (getgrouplist(pw->pw_name, pw->pw_gid, nullptr, &ngroups) == -1) {
            //            log_error() << "getgrouplist() returned -1; ngroups = %d\n" << ngroups;
        }

        groups.resize(ngroups);

        if (getgrouplist(pw->pw_name, pw->pw_gid, groups.data(), &ngroups) == -1) {
            //log_error() << "getgrouplist() returned -1; ngroups = %d\n" << ngroups;
            return ReturnCode::FAILURE;
        }

        StringBuilder s;
        for (auto &gid : groups) {
            s << " " << gid;
        }

        //log_debug() << "setuid(" << userID << ")" << "  setgid(" << pw->pw_gid << ")  " << "Groups " << s.str();

        if (setgroups(groups.size(), groups.data()) != 0) {
            //log_error() << "setgroups failed";
            return ReturnCode::FAILURE;
        }

        if (setgid(pw->pw_gid) != 0) {
            //log_error() << "setgid failed";
            return ReturnCode::FAILURE;
        }

        if (setuid(userID) != 0) {
            //log_error() << "setuid failed";
            return ReturnCode::FAILURE;
        }

    } else {
        //log_error() << "Error in getpwuid";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;

}

ReturnCode Container::attach(const std::string &commandLine, pid_t *pid, uid_t userID)
{
    return attach(commandLine, pid, m_gatewayEnvironmentVariables, userID);
}

ReturnCode Container::attach(const std::string &commandLine, pid_t *pid, const EnvironmentVariables &variables, uid_t userID,
        const std::string &workingDirectory, int stdin, int stdout,
        int stderr)
{
    if (isError(ensureContainerRunning())) {
        log_error() << "Container is not running or in bad state, can't attach";
        return ReturnCode::FAILURE;
    }

    if (pid == nullptr) {
        log_error() << "Supplied pid argument is nullptr";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Attach " << commandLine << " UserID:" << userID;

    std::vector<std::string> executeCommandVec = Glib::shell_parse_argv(commandLine);
    std::vector<char *> args;

    for (size_t i = 0; i < executeCommandVec.size(); i++) {
        executeCommandVec[i].c_str(); // ensure the string is null-terminated. not sure thas is required.
        auto s = &executeCommandVec[i][0];
        args.push_back(s);
    }

    // We need a null terminated array
    args.push_back(nullptr);

    // We execute the function as root but will switch to the real userID inside

    ReturnCode result = executeInContainer([&] () {
//                log_debug() << "Starting command line in container : " << commandLine << " . Working directory : " << workingDirectory;

                if (!isSuccess(setUser(userID))) {
                    return -1;
                }

                if (workingDirectory.length() != 0) {
                    auto ret = chdir(workingDirectory.c_str());
                    if (ret != 0) {
//                        log_error() << "Error when changing current directory : " << strerror(errno);
                    }

                }
                execvp(args[0], args.data());

//                log_error() << "Error when executing the command in container : " << strerror(errno);
                return 1;
            }, pid, variables, ROOT_UID, stdin, stdout, stderr);
    if (isError(result)) {
        log_error() << "Could not execute in container";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

ReturnCode Container::stop()
{
    ReturnCode ret = ReturnCode::SUCCESS;
    if (m_state >= ContainerState::STARTED) {
        log_debug() << "Stopping the container";
        if (m_container->stop(m_container)) {
            log_debug() << "Container stopped, waiting for stop state";
            waitForState(LXCContainerState::STOPPED);
        } else {
            log_error() << "Unable to stop container";
            ret = ReturnCode::FAILURE;
        }
    } else {
        log_error() << "Can't stop container that has not been started";
        ret = ReturnCode::FAILURE;
    }

    return ret;
}

ReturnCode Container::shutdown()
{
    return shutdown(m_shutdownTimeout);
}

ReturnCode Container::shutdown(unsigned int timeout)
{
    if (m_state < ContainerState::STARTED) {
        log_error() << "Trying to shutdown container that has not been started. Aborting";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Shutting down container " << toString() << " pid: " << m_container->init_pid(m_container);

    if (m_container->init_pid(m_container) != INVALID_PID) {
        kill(m_container->init_pid(m_container), SIGTERM);
    }

    // Shutdown with timeout
    bool success = m_container->shutdown(m_container, timeout);
    if (!success) {
        log_warning() << "Failed to cleanly shutdown container, forcing stop" << toString();
        if(isError(stop())) {
            log_error() << "Failed to force stop the container" << toString();
            return ReturnCode::FAILURE;
        }
    }

    m_state = ContainerState::CREATED;
    return ReturnCode::SUCCESS;
}

ReturnCode Container::destroy()
{
    return destroy(m_shutdownTimeout);
}

ReturnCode Container::destroy(unsigned int timeout)
{
    if (m_state < ContainerState::CREATED) {
        log_error() << "Trying to destroy container that has not been created. Aborting destroy";
        return ReturnCode::FAILURE;
    }

    if (m_state >= ContainerState::STARTED) {
        if(isError(shutdown(timeout))) {
            log_error() << "Could not shutdown container. Aborting destroy";
            return ReturnCode::FAILURE;
        }
    }

    // Destroy it!
    bool success = m_container->destroy(m_container);
    if (!success) {
        log_error() << "Failed to destroy the container " << toString();
        return ReturnCode::FAILURE;
    }

    m_state = ContainerState::DESTROYED;
    return ReturnCode::SUCCESS;
}


ReturnCode Container::bindMountFileInContainer(const std::string &pathOnHost,
                                               const std::string &pathInContainer,
                                               std::string &result,
                                               bool readonly)
{
    if(isError(ensureContainerRunning())) {
        log_error() << "Container is not running or in bad state, can't bind-mount file";
        return ReturnCode::FAILURE;
    }

    std::string dst = gatewaysDir() + "/" + pathInContainer;

    if (isError(touch(dst))) {
        log_error() << "Could not create " << dst;
        return ReturnCode::FAILURE;
    }
    m_cleanupHandlers.push(new FileCleanUpHandler(dst));

    if (isError(bindMount(pathOnHost, dst, readonly))) {
        log_error() << "Could not bind mount " << pathOnHost << " to " << dst;
        return ReturnCode::FAILURE;
    }

    result = gatewaysDirInContainer() + "/" + pathInContainer;
    return ReturnCode::SUCCESS;
}

ReturnCode Container::bindMountFolderInContainer(const std::string &pathOnHost,
                                                 const std::string &pathInContainer,
                                                 std::string &result,
                                                 bool readonly)
{
    if(isError(ensureContainerRunning())) {
        log_error() << "Container is not running or in bad state, can't bind-mount folder";
        return ReturnCode::FAILURE;
    }

    std::string dst = gatewaysDir() + "/" + pathInContainer;

    log_debug() << "Creating folder : " << dst;
    if (isError(createDirectory(dst))) {
        log_error() << "Could not create folder " << dst;
        return ReturnCode::FAILURE;
    }

    if (isError(bindMount(pathOnHost, dst, readonly, m_enableWriteBuffer))) {
        log_error() << "Could not bind mount " << pathOnHost << " to " << dst;
        return ReturnCode::FAILURE;
    }

    result = gatewaysDirInContainer() + "/" + pathInContainer;
    return ReturnCode::SUCCESS;
}

ReturnCode Container::mountDevice(const std::string &pathInHost)
{
    if(isError(ensureContainerRunning())) {
        log_error() << "Container is not running or in bad state, can't mount device";
        return ReturnCode::FAILURE;
    }
    log_debug() << "Mounting device in container : " << pathInHost;
    bool returnCode = m_container->add_device_node(m_container, pathInHost.c_str(), nullptr);
    return bool2ReturnCode(returnCode);
}

ReturnCode Container::executeInContainer(const std::string &cmd)
{
    if (isError(ensureContainerRunning())) {
        log_error() << "Container is not running or in bad state, can't execute in container";
        return ReturnCode::FAILURE;
    }

    pid_t pid = INVALID_PID;
    ReturnCode result = executeInContainer([this, cmd]() {
                //log_info() << "Executing system command in container : " << cmd;
                const int result = execl("/bin/sh", "sh", "-c", cmd.c_str(), 0);
                //log_debug() << "Excution of system command " << cmd << " resulted in status " << result;
                return result;
            }, &pid, m_gatewayEnvironmentVariables);

    if (isSuccess(result)) {
        const int commandResponse = waitForProcessTermination(pid);
        if (commandResponse != 0) {
            log_debug() << "Exectution of command " << cmd << " in container failed";
            return ReturnCode::FAILURE;
        }
    } else {
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

ReturnCode Container::setEnvironmentVariable(const std::string &var, const std::string &val)
{
    if (m_state < ContainerState::CREATED) {
        log_error() << "Can't set environment variable for non-created container";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Setting env variable in container " << var << "=" << val;
    m_gatewayEnvironmentVariables[var] = val;

    // We generate a file containing all variables for convenience when connecting to the container in command-line
    StringBuilder s;
    for (auto &var : m_gatewayEnvironmentVariables) {
        s << "export " << var.first << "='" << var.second << "'\n";
    }
    std::string path = gatewaysDir() + "/env";
    FileToolkitWithUndo::writeToFile(path, s);

    return ReturnCode::SUCCESS;
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
    return m_containerRoot + "/" + id() + GATEWAYS_PATH;
}

const std::string &Container::rootFS() const
{
    return m_rootFSPath;
}
