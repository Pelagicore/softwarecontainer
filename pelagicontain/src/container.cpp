/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include <fstream>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <lxc/lxccontainer.h>
#include <lxc/version.h>
#include "lxc-common.h"

#include <pwd.h>
#include <grp.h>

#include "container.h"
#include "pelagicore-common.h"

std::vector<const char *> Container::s_LXCContainerStates;
const char *Container::s_LXCRoot;

void Container::init_lxc()
{
    static bool bInitialized = false;
    if (!bInitialized) {
        int stateCount = lxc_get_wait_states(nullptr);
        s_LXCContainerStates.resize(stateCount);
        lxc_get_wait_states( s_LXCContainerStates.data() );
        assert( (int)LXCContainerState::ELEMENT_COUNT == s_LXCContainerStates.size() );
        bInitialized = true;
        s_LXCRoot = lxc_get_global_config_item(LXC_CONTAINERS_ROOT_CONFIG_ITEM);
    }
}

/* A directory need to exist and be set up in a special way.
 *
 * Basically something like:
 * mkdir -p <containerRoot>/late_mounts
 * mount --bind <containerRoot>/late_mounts <containerRoot>/late_mounts
 * mount --make-unbindable <containerRoot>/late_mounts
 * mount --make-shared <containerRoot>/late_mounts
 */

Container::Container(const std::string &id, const std::string &name, const std::string &configFile,
        const std::string &containerRoot) :
    m_configFile(configFile),
    m_id(id),
    m_name(name),
    m_containerRoot(containerRoot)
{
    init_lxc();
}

ReturnCode Container::initialize()
{
    std::string gatewayDir = gatewaysDir();
    if ( isError( createDirectory(gatewayDir) ) ) {
        log_error() << "Could not create gateway directory " << gatewayDir << strerror(errno);
    }

    // Make sure the directory for "late mounts" exists
    if ( !isDirectory( lateMountDir() ) ) {
        log_error() << "Directory " << lateMountDir() << " does not exist";
        return ReturnCode::FAILURE;
    }

    // Create directories needed for mounting, will be removed in dtor
    bool allOk = true;
    allOk = allOk && isSuccess( createDirectory( applicationMountDir() ) );
    allOk = allOk && isSuccess( createDirectory(applicationMountDir() + "/bin") );
    allOk = allOk && isSuccess( createDirectory(applicationMountDir() + "/shared") );
    allOk = allOk && isSuccess( createDirectory(applicationMountDir() + "/home") );

    if (!allOk) {
        log_error() << "Could not set up all needed directories";
    }

    createSharedMountPoint(gatewayDir);

    m_initialized = allOk;

    return allOk ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}


Container::~Container()
{
    if ( isContainerDeleteEnabled() ) {
        destroy();
        if (m_container != nullptr) {
            lxc_container_put(m_container);
            m_container = nullptr;
        }
    }

}

std::string Container::toString()
{
    std::stringstream ss;
    ss << "LXC " << id() << " ";
    if (m_container != nullptr) {
        ss << "id: " << id()
           << "name : " << m_name
           << " / state:" << m_container->state(m_container)
           << " / initPID:" << m_container->init_pid(m_container)
           << " / LastError: " << m_container->error_string
           << " / To connect to this container : lxc-attach -n " << id();
    }

    return ss.str();
}

void Container::create()
{
    log_debug() << "Creating container " << toString();

    const char *containerID = id();

    std::string containerPath = m_containerRoot;

    setenv("GATEWAY_DIR", (gatewaysDir() + "/").c_str(), true);
    setenv("MOUNT_DIR", (containerPath + LATE_MOUNT_PATH).c_str(), true);

    log_debug() << "GATEWAY_DIR : " << getenv("GATEWAY_DIR");
    log_debug() << "MOUNT_DIR : " << getenv("MOUNT_DIR");

    auto configFile = m_configFile.c_str();
    log_debug() << "Config file : " << configFile;
    log_debug() << "Template : " << LXCTEMPLATE;

    m_container = lxc_container_new(containerID, nullptr);

    lxc_container_get(m_container);

    log_debug() << toString();

    char *argv[] = {"lxc-create", nullptr};

    int flags = 0;

    struct bdev_specs specs = {};

    m_container->load_config(m_container, configFile);

    m_container->create(m_container, LXCTEMPLATE, nullptr, &specs, flags, argv);

    m_rootFSPath = (StringBuilder() << s_LXCRoot << "/" << containerID << "/rootfs");

    log_debug() << "Container created. RootFS: " << m_rootFSPath;

}

void Container::waitForState(LXCContainerState state, int timeout)
{
    if ( strcmp( m_container->state(m_container), toString(state) ) ) {
        log_debug() << "Waiting for container to change to state : " << toString(state);
        bool b = m_container->wait(m_container, toString(state), timeout);
        log_debug() << toString() << " " << b;
    }
}

pid_t Container::start()
{
    pid_t pid;

    if (isLXC_C_APIEnabled() && false) {

        // TODO : check why starting the container via the C API fails here. We still use the command-line for that reason

        log_debug() << "Starting container";

        const char *argv[] = {"/bin/sleep", nullptr};
        char *emptyArgv[] = {"100000000", nullptr};
        m_container->start(m_container, true, emptyArgv);

        log_debug() << "Container started__ : " << toString();

        pid = m_container->init_pid(m_container);

    } else {
        std::vector<std::string> executeCommandVec;
        std::string lxcCommand = StringBuilder() << "lxc-execute -n " << id() << " -- env " << "/bin/sleep 100000000";
        executeCommandVec = Glib::shell_parse_argv(lxcCommand);
        std::vector<std::string> envVarVec = {"MOUNT_DIR=" + lateMountDir()};

        log_debug() << "Execute: " << lxcCommand;

        try {
            Glib::spawn_async_with_pipes(
                    ".",
                    executeCommandVec,
                    envVarVec,
                    Glib::SPAWN_DO_NOT_REAP_CHILD | Glib::SPAWN_SEARCH_PATH,
                    sigc::slot<void>(),
                    &pid);
        } catch (const Glib::Error &ex) {
            log_error() << "spawn error: " << ex.what();
            pid = 0;
        }
    }

    //    assert( m_container->is_running(m_container) );

    log_info() << "To connect to this container : lxc-attach -n " << id();

    return pid;

}

int Container::executeInContainerEntryFunction(void *param)
{
    ContainerFunction *function = (ContainerFunction *) param;
    return (*function)();
}

pid_t Container::executeInContainer(ContainerFunction function, const EnvironmentVariables &variables, uid_t userID, int stdin,
        int stdout,
        int stderr)
{
    ensureContainerRunning();

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
            if ( m_gatewayEnvironmentVariables.at(var.first) != variables.at(var.first) ) {
                log_warning() << "Variable set by gateway overriding original variable value: " << var.first << ". values: " <<
                var.second;
            }
        }

        actualVariables[var.first] = var.second;
    }

    // prepare array of env variable strings to be set when launching the process in the container
    std::vector<std::string> strings;
    for (auto &var : actualVariables) {
        strings.push_back( pelagicore::formatString( "%s=%s", var.first.c_str(), var.second.c_str() ) );
    }

    const char *envVariablesArray[strings.size() + 1];
    for (size_t i = 0; i < strings.size(); i++) {
        envVariablesArray[i] = strings[i].c_str();
    }
    envVariablesArray[strings.size()] = nullptr;
    options.extra_env_vars = (char * *) envVariablesArray;    // TODO : get LXC fixed so that extra_env_vars points to an array of const char* instead of char*

    log_debug() << "Starting function in container " << toString() << "User:" << userID << "" << std::endl <<
    " Env variables : " << strings;

    pid_t attached_process_pid = 0;

    m_container->attach(m_container, &Container::executeInContainerEntryFunction, &function, &options, &attached_process_pid);

    log_info() << " Attached PID: " << attached_process_pid;

    if (attached_process_pid == 0) {
        log_error() << "PID = 0";
    }

    //    assert(attached_process_pid != 0);

    return attached_process_pid;
}

pid_t Container::attach(const std::string &commandLine, uid_t userID)
{
    return attach(commandLine, m_gatewayEnvironmentVariables, userID);
}

ReturnCode Container::setUser(uid_t userID)
{
    log_info() << "Setting env for userID : " << userID;

    struct passwd *pw = getpwuid(userID);
    if (pw != nullptr) {
        std::vector<gid_t> groups;
        int ngroups = 0;

        if (getgrouplist(pw->pw_name, pw->pw_gid, nullptr, &ngroups) == -1) {
            //            log_error() << "getgrouplist() returned -1; ngroups = %d\n" << ngroups;
        }

        groups.resize(ngroups);

        if (getgrouplist(pw->pw_name, pw->pw_gid, groups.data(), &ngroups) == -1) {
            log_error() << "getgrouplist() returned -1; ngroups = %d\n" << ngroups;
            return ReturnCode::FAILURE;
        }

        StringBuilder s;
        for (auto &gid : groups) {
            s << " " << gid;
        }

        log_debug() << "setuid(" << userID << ")" << "  setgid(" << pw->pw_gid << ")  " << "Groups " << s.str();

        if (setgroups( groups.size(), groups.data() ) != 0) {
            log_error() << "setgroups failed";
            return ReturnCode::FAILURE;
        }

        if (setgid(pw->pw_gid) != 0) {
            log_error() << "setgid failed";
            return ReturnCode::FAILURE;
        }

        if (setuid(userID) != 0) {
            log_error() << "setuid failed";
            return ReturnCode::FAILURE;
        }

    } else {
        log_error() << "Error in getpwuid";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;

}

pid_t Container::attach(const std::string &commandLine, const EnvironmentVariables &variables, uid_t userID,
        const std::string &workingDirectory, int stdin, int stdout,
        int stderr)
{
    ensureContainerRunning();

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

    //	for(size_t i=0; i <= executeCommandVec.size(); i++)	log_debug() << args[i];

    // We execute the function as root but will switch to the real userID inside
    return executeInContainer([&] () {

                log_debug() << "Starting command line in container : " << commandLine << " . Working directory : " <<
                workingDirectory;

                if ( !isSuccess( setUser(userID) ) ) {
                    return -1;
                }

                if (workingDirectory.length() != 0) {
                    auto ret = chdir( workingDirectory.c_str() );
                    if (ret != 0) {
                        log_error() << "Error when changing current directory : " << strerror(errno);
                    }

                }
                execvp( args[0], args.data() );

                log_error() << "Error when executing the command in container : " << strerror(errno);

                return 1;

            }, variables, ROOT_UID, stdin, stdout, stderr);

}

void Container::stop()
{
    log_debug() << "Stopping the container";

    if (m_container != nullptr) {
        m_container->stop(m_container);
        waitForState(LXCContainerState::STOPPED);
    }

}

void Container::destroy()
{
    stop();

    log_debug() << "Shutting down container " << toString() << " pid " << m_container->init_pid(m_container);

    if (m_container->init_pid(m_container) > 1) {
        kill(m_container->init_pid(m_container), SIGTERM);
    }

    auto timeout = 2;
    m_container->shutdown(m_container, timeout);

}


std::string Container::bindMountFileInContainer(const std::string &pathOnHost, const std::string &pathInContainer,
        bool readonly)
{
    ensureContainerRunning();

    std::string dst = gatewaysDir() + "/" + pathInContainer;

    touch(dst);
    m_cleanupHandlers.push_back( new FileCleanUpHandler(dst) );
    auto s = bindMount(pathOnHost, dst, readonly);

    std::string actualPathInContainer = gatewaysDirInContainer();
    actualPathInContainer += +"/" + pathInContainer;

    return actualPathInContainer;
}

std::string Container::bindMountFolderInContainer(const std::string &pathOnHost, const std::string &pathInContainer,
        bool readonly)
{
    ensureContainerRunning();

    std::string dst = gatewaysDir() + "/" + pathInContainer;

    log_debug() << "Creating folder : " << dst;
    createDirectory(dst);
//    mkdir(dst.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
//    m_cleanupHandlers.push_back( new DirectoryCleanUpHandler(dst) );

    auto s = bindMount(pathOnHost, dst, readonly);

    return gatewaysDirInContainer() + "/" + pathInContainer;
}

ReturnCode Container::bindMount(const std::string &src, const std::string &dst, bool readOnly)
{
    ensureContainerRunning();
    return FileToolkitWithUndo::bindMount(src, dst, readOnly);
}

ReturnCode Container::mountDevice(const std::string &pathInHost)
{
    ensureContainerRunning();
    log_debug() << "Mounting device in container : " << pathInHost;
    auto returnCode = m_container->add_device_node(m_container, pathInHost.c_str(), nullptr);
    return (returnCode) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}

bool Container::mountApplication(const std::string &appDirBase)
{
    bool allOk = true;
    allOk &= isSuccess( bindMount(appDirBase + "/bin", applicationMountDir() + "/bin") );
    allOk &= isSuccess( bindMount(appDirBase + "/shared", applicationMountDir() + "/shared") );
    allOk &= isSuccess( bindMount(appDirBase + "/home", applicationMountDir() + "/home") );

    allOk = true;

    return allOk;
}

ReturnCode Container::executeInContainer(const std::string &cmd)
{
    ensureContainerRunning();

    pid_t pid = executeInContainer([this, cmd = cmd]() {
                log_info() << "Executing system command in container : " << cmd;
                return system( cmd.c_str() );
            }, m_gatewayEnvironmentVariables);

    waitForProcessTermination(pid);

    return ReturnCode::SUCCESS;
}

ReturnCode Container::setEnvironmentVariable(const std::string &var, const std::string &val)
{
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
