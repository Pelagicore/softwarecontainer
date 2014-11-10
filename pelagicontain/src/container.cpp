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

#include "container.h"
#include "pelagicore-common.h"

#ifndef LXCTEMPLATE
    #error Must define LXCTEMPLATE as path to lxc-pelagicontain
#endif

class DirectoryCleanUpHandler :
    public Container::CleanUpHandler
{
public:
    DirectoryCleanUpHandler(const std::string &path)
    {
        m_path = path;
    }

    ReturnCode clean() override
    {
        auto code = ReturnCode::FAILURE;

        if (rmdir( m_path.c_str() ) == 0) {
            code = ReturnCode::SUCCESS;
        } else {
            log_error() << "Can't rmdir " << m_path << " . Error :" << strerror(errno);
        }

        return code;
    }

    std::string m_path;
};

class FileCleanUpHandler :
    public Container::CleanUpHandler
{
public:
    FileCleanUpHandler(const std::string &path)
    {
        m_path = path;
    }

    ReturnCode clean() override
    {
        auto code = ReturnCode::FAILURE;

        if (unlink( m_path.c_str() ) == 0) {
            code = ReturnCode::SUCCESS;
        } else {
            log_error() << "Can't delete " << m_path << " . Error :" << strerror(errno);
        }

        return code;
    }

    std::string m_path;
};

class MountCleanUpHandler :
    public Container::CleanUpHandler
{
public:
    MountCleanUpHandler(const std::string &path)
    {
        m_path = path;
    }

    ReturnCode clean() override
    {
        auto code = ReturnCode::FAILURE;

        if (umount( m_path.c_str() ) == 0) {
            code = ReturnCode::SUCCESS;
        } else {
            log_error() << "Can't unmount " << m_path << " . Error :" << strerror(errno);
        }

        return code;
    }

    std::string m_path;
};

/* A directory need to exist and be set up in a special way.
 *
 * Basically something like:
 * mkdir -p <containerRoot>/late_mounts
 * mount --bind <containerRoot>/late_mounts <containerRoot>/late_mounts
 * mount --make-unbindable <containerRoot>/late_mounts
 * mount --make-shared <containerRoot>/late_mounts
 */

Container::Container(const std::string &name, const std::string &configFile, const std::string &containerRoot) :
    m_configFile(configFile),
    m_name(name),
    m_containerRoot(containerRoot)
{
    log_debug() << "LXC version " << lxc_get_version();
    int stateCount = lxc_get_wait_states(nullptr);
    m_LXCContainerStates.resize(stateCount);
    lxc_get_wait_states( m_LXCContainerStates.data() );
    assert( (int)LXCContainerStates::ELEMENT_COUNT == m_LXCContainerStates.size() );
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

    auto mountRes = mount(gatewayDir.c_str(), gatewayDir.c_str(), "", MS_BIND, NULL);
    assert(mountRes == 0);
    mountRes = mount(gatewayDir.c_str(), gatewayDir.c_str(), "", MS_UNBINDABLE, NULL);
    assert(mountRes == 0);
    mountRes = mount(gatewayDir.c_str(), gatewayDir.c_str(), "", MS_SHARED, NULL);
    assert(mountRes == 0);
    m_cleanupHandlers.push_back( new MountCleanUpHandler(gatewayDir) );

    m_initialized = allOk;

    return allOk ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}

ReturnCode Container::createDirectory(const std::string &path)
{
    if ( isDirectory(path) ) {
        return ReturnCode::SUCCESS;
    }

    auto parent = parentPath(path);
    if ( !isDirectory(parent) ) {
        createDirectory(parent);
    }

    if (mkdir(path.c_str(), S_IRWXU) == -1) {
        log_error() << "Could not create directory " << path << " / " << strerror(errno);
        return ReturnCode::FAILURE;
    }

    m_cleanupHandlers.push_back( new DirectoryCleanUpHandler(path) );
    log_debug() << "Created directory " << path;

    return ReturnCode::SUCCESS;
}


Container::~Container()
{
    // Clean up all created directories, files, and mount points
    for (auto it = m_cleanupHandlers.rbegin(); it != m_cleanupHandlers.rend(); ++it) {
        if ( !isSuccess( (*it)->clean() ) ) {
            log_error() << "Error";
        }
        delete *it;
    }

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
    ss << "LXCContainer "
       << "name: " << m_container->name
       << " / state:" << m_container->state(m_container)
       << " / initPID:" << m_container->init_pid(m_container)
       << " / LastError: " << m_container->error_string;

    return ss.str();
}

void Container::create()
{
    const char *containerName = name();

    std::string containerPath = m_containerRoot;

    setenv("GATEWAY_DIR", (gatewaysDir() + "/").c_str(), true);
    setenv("MOUNT_DIR", (containerPath + LATE_MOUNT_PATH).c_str(), true);

    log_debug() << "GATEWAY_DIR : " << getenv("GATEWAY_DIR");
    log_debug() << "MOUNT_DIR : " << getenv("MOUNT_DIR");

    auto configFile = m_configFile.c_str();
    log_debug() << "Config file : " << configFile;
    log_debug() << "Template : " << LXCTEMPLATE;

    m_container = lxc_container_new(containerName, nullptr);

    lxc_container_get(m_container);

    log_debug() << toString();

    char *argv[] = {"lxc-create", nullptr};

    int flags = 0;

    struct bdev_specs specs = {};

    m_container->load_config(m_container, configFile);

    m_container->create(m_container, LXCTEMPLATE, nullptr, &specs, flags, argv);

    log_debug() << "Container created";

}

pid_t Container::start()
{
    pid_t pid;

    if (isLXC_C_APIEnabled() && false) {

        // TODO : check why starting the container via the C API fails here. We still use the command-line for that reason

        log_debug() << "Starting container";

        char *argv[] = {"/bin/sleep", nullptr};
        char *emptyArgv[] = {"100000000", nullptr};
        m_container->start(m_container, true, emptyArgv);

        log_debug() << "Container started : " << toString();

        assert( m_container->is_running(m_container) );

        pid = m_container->init_pid(m_container);

    } else {
        std::vector<std::string> executeCommandVec;
        std::string lxcCommand = StringBuilder() << "lxc-execute -n " << name() << " -- env " << "/bin/sleep 100000000";
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

    return pid;

}

int Container::executeInContainerEntryFunction(void *param)
{
    ContainerFunction *function = (ContainerFunction *) param;
    return (*function)();
}

pid_t Container::executeInContainer(ContainerFunction function, const EnvironmentVariables &variables, int stdin, int stdout,
        int stderr)
{
    lxc_attach_options_t options = LXC_ATTACH_OPTIONS_DEFAULT;
    options.stdin_fd = stdin;
    options.stdout_fd = stdout;
    options.stderr_fd = stderr;

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
    options.extra_env_vars = (char * *) envVariablesArray;  // TODO : get LXC fixed so that extra_env_vars points to an array of const char* instead of char*

    log_debug() << "Env variables : " << strings;

    pid_t attached_process_pid = 0;

    m_container->attach(m_container, &Container::executeInContainerEntryFunction, &function, &options, &attached_process_pid);

    log_info() << " Attached PID: " << attached_process_pid;

    assert(attached_process_pid != 0);

    return attached_process_pid;
}

pid_t Container::attach(const std::string &commandLine)
{
    return attach(commandLine, m_gatewayEnvironmentVariables);
}

pid_t Container::attach(const std::string &commandLine, const EnvironmentVariables &variables, int stdin, int stdout, int stderr,
        const std::string &workingDirectory)
{

    log_debug() << "Attach " << commandLine;

    std::vector<std::string> executeCommandVec = Glib::shell_parse_argv(commandLine);
    const char *args[executeCommandVec.size() + 1];

    for (size_t i = 0; i < executeCommandVec.size(); i++) {
        args[i] = executeCommandVec[i].c_str();
    }

    args[executeCommandVec.size()] = nullptr;

    //	for(size_t i=0; i <= executeCommandVec.size(); i++)	log_debug() << args[i];

    return executeInContainer([&] () {

                log_debug() << "Starting command line in container : " << commandLine <<
                " . Working directory : " << workingDirectory;

                if (workingDirectory.length() != 0) {
                    auto ret = chdir( workingDirectory.c_str() );
                    if (ret != 0) {
                        log_error() << "Error when changing current directory : " << strerror(errno);
                    }

                }
                execvp(args[0], (char *const *) args);

                log_error() << "Error when executing the command in container : " << strerror(errno);

                return 1;

            }, variables, stdin, stdout, stderr);

}

void Container::stop()
{
    log_debug() << "Stopping the container";

    if (m_container != nullptr) {
        m_container->stop(m_container);
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
    std::string dst = gatewaysDir() + "/" + pathInContainer;

    log_debug() << "Creating folder : " << dst;
    mkdir(dst.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    m_cleanupHandlers.push_back( new DirectoryCleanUpHandler(dst) );
    auto s = bindMount(pathOnHost, dst, readonly);

    return gatewaysDirInContainer() + "/" + pathInContainer;
}

ReturnCode Container::bindMount(const std::string &src, const std::string &dst, bool readOnly)
{
    int flags = MS_BIND;

    if (readOnly) {
        flags |= MS_RDONLY;
    }

    log_debug() << "Mounting " << (readOnly ? " readonly " : "read/write ") << src << " in " << dst << " / flags: " << flags;

    int mountRes = mount(src.c_str(), // source
            dst.c_str(),              // target
            "",                       // fstype
            flags,                  // flags
            NULL);                    // data

    auto result = ReturnCode::FAILURE;

    if (mountRes == 0) {
        // Success
        m_cleanupHandlers.push_back( new MountCleanUpHandler(dst) );

        log_verbose() << "Mounted folder " << src << " in " << dst;
        result = ReturnCode::SUCCESS;
    } else {
        // Failure
        log_error( "Could not mount into container: src=%s, dst=%s err=%s",
                src.c_str(),
                dst.c_str(),
                strerror(errno) );
    }

    return result;
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

ReturnCode Container::mountDevice(const std::string &pathInHost)
{
    log_debug() << "Mounting device in container : " << pathInHost;
    auto returnCode = m_container->add_device_node(m_container, pathInHost.c_str(), nullptr);
    return (returnCode) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}


ReturnCode Container::systemCall(const std::string &cmd)
{

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
    return ReturnCode::SUCCESS;
}
