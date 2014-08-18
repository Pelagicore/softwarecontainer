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

#include "container.h"

#ifndef LXCTEMPLATE
    #error Must define LXCTEMPLATE as path to lxc-pelagicontain
#endif

/* A directory need to exist and be set up in a special way.
 *
 * Basically something like:
 * mkdir -p <containerRoot>/late_mounts
 * mount --bind <containerRoot>/late_mounts <containerRoot>/late_mounts
 * mount --make-unbindable <containerRoot>/late_mounts
 * mount --make-shared <containerRoot>/late_mounts
 */

Container::Container(const std::string &name,
                     const std::string &configFile,
                     const std::string &containerRoot):
    m_configFile(configFile),
    m_name(name),
    m_containerRoot(containerRoot),
    m_mountDir(containerRoot + "/late_mounts")
{
	log_debug() << "LXC version " << lxc_get_version();
}

ReturnCode Container::initialize()
{
    // Make sure the directory for "late mounts" exist
    if (!isDirectory(m_mountDir)) {
        log_error() << "Directory " << m_mountDir << " does not exist";
        return ReturnCode::FAILURE;
    }

    // Create directories needed for mounting, will be removed in dtor
    std::string runDir = m_mountDir + "/" + m_name;

    bool allOk = true;
    allOk = allOk && isSuccess(createDirectory(runDir.c_str()));
    allOk = allOk && isSuccess(createDirectory((runDir + "/bin").c_str()));
    allOk = allOk && isSuccess(createDirectory((runDir + "/shared").c_str()));
    allOk = allOk && isSuccess(createDirectory((runDir + "/home").c_str()));

    if (!allOk)
        log_error() << "Could not set up all needed directories";

    return allOk ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
}

ReturnCode Container::createDirectory(const std::string &path)
{
    if (mkdir(path.c_str(), S_IRWXU) == -1) {
        log_error() << "Could not create directory " << path << " / " << strerror(errno);
        return ReturnCode::FAILURE;
    }

	log_debug() << "Created directory " << path;

    m_dirs.push_back(path);
    return ReturnCode::SUCCESS;
}

bool Container::isDirectory(const std::string &path)
{
    bool isDir = false;
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        if ((st.st_mode & S_IFDIR) != 0) {
            isDir = true;
        }
    }
    return isDir;
}

Container::~Container()
{
    // Unmount all mounted dirs. Done backwards, behaving like a stack.
    for (auto it = m_mounts.rbegin(); it != m_mounts.rend(); ++it)
    {
        log_debug() << "Unmounting " << *it;
        if (umount((*it).c_str()) == -1) {
            log_error() << "Could not unmount " << *it << strerror(errno);
        }
    }

    // Clean up all created directories, also done backwards
    for (auto it = m_dirs.rbegin(); it != m_dirs.rend();++it)
    {
        log_debug() << "Removing " << (*it).c_str();
        if (rmdir((*it).c_str()) == -1) {
            log_error() << "Could not remove dir " << *it << strerror(errno);
        }
    }

	if (isContainerDeleteEnabled())
		destroy();

	if (m_container != nullptr)
		lxc_container_put(m_container);

	m_container = nullptr;
}

const char *Container::name()
{
    return m_name.c_str();
}

std::string  Container::toString() {
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
if (isLXC_C_APIEnabled()) {
	const char* containerName = name();

	std::string containerPath = m_containerRoot;

	setenv("CONTROLLER_DIR", (containerPath + "/bin").c_str(), true);
	setenv("GATEWAY_DIR", (containerPath + "/" + containerName + "/gateways/").c_str(), true);
	setenv("MOUNT_DIR", (containerPath + "/late_mounts").c_str(), true);

	log_debug() << "CONTROLLER_DIR : " << getenv("CONTROLLER_DIR");
	log_debug() << "GATEWAY_DIR : " << getenv("GATEWAY_DIR");
	log_debug() << "MOUNT_DIR : " << getenv("MOUNT_DIR");

	auto configFile = m_configFile.c_str();
	log_debug() << "Config file : " << configFile;
	log_debug() << "Template : " << LXCTEMPLATE;

	m_container = lxc_container_new(containerName, nullptr);

	lxc_container_get(m_container);

	log_debug() << toString();

	char* argv[] = { "lxc-create", nullptr };

	int flags = 0;

	struct bdev_specs specs = {};

	m_container->load_config(m_container, configFile);

	m_container->create(m_container, LXCTEMPLATE, nullptr, &specs, flags, argv);

} else {

    int maxCmdLen = sysconf(_SC_ARG_MAX);
    char lxcCommand[maxCmdLen];

    // Command to create container
    sprintf(lxcCommand,
            "CONTROLLER_DIR=%s GATEWAY_DIR=%s MOUNT_DIR=%s lxc-create -n %s -t %s"
            " -f %s > /tmp/lxc_%s.log",
            (m_containerRoot + "/bin").c_str(),
            (m_containerRoot + name() + "/gateways/").c_str(),
            m_mountDir.c_str(),
            name(),
            LXCTEMPLATE,
            m_configFile.c_str(),
            name());

    log_debug() << "Command " << &lxcCommand[0];
    system(lxcCommand);
}

}

pid_t Container::start()
{

	if(isLXC_C_APIEnabled() && false){
		log_debug() << "Starting container";

		char* argv[] = { "/controller/controller", nullptr};
//		m_container->start(m_container, false, argv );
		m_container->startl(m_container, false, argv[0], nullptr );

		log_debug() << "Container started : " << toString();

		assert(m_container->is_running(m_container));

		return m_container->init_pid(m_container);

} else {
	int maxCmdLen = sysconf(_SC_ARG_MAX);
    char lxcCommand[maxCmdLen];

    // Create command to execute inside container
    snprintf(lxcCommand, sizeof(lxcCommand),
             "lxc-execute -n %s -- env %s",
             name(),
             "/controller/controller");

    std::vector<std::string> executeCommandVec;
    executeCommandVec = Glib::shell_parse_argv(std::string(lxcCommand));
    std::vector<std::string> envVarVec = {"MOUNT_DIR=" + m_mountDir};

    log_debug() << "Execute: " << &lxcCommand[0];

    pid_t pid;
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

    return pid;

}

}

int Container::executeInContainerEntryFunction(void* param) {
	ContainerFunction* function = (ContainerFunction*) param;
	(*function)();
	return 0;
}

pid_t Container::executeInContainer(ContainerFunction function) {
	lxc_attach_options_t options = LXC_ATTACH_OPTIONS_DEFAULT;
	options.stdin_fd = -1;  // no stdin
	options.stdout_fd = 1;
	options.stderr_fd = 2;
	pid_t attached_process_pid = 0;
	m_container->attach(m_container, &Container::executeInContainerEntryFunction, &function, &options, &attached_process_pid);
	log_info() << " Attached PID: " << attached_process_pid;
	return attached_process_pid;
}

pid_t Container::attach(const std::string& commandLine) {

if(isLXC_C_APIEnabled()) {
		log_debug() << "Attach " << commandLine;

		return executeInContainer([&] () {

//			std::vector<std::string> envVarVec = {};
//			pid_t pid;

			// TODO: set environment

			execl(commandLine.c_str(), commandLine.c_str(), nullptr);

//			GError* error = nullptr;
//			if ( g_spawn_command_line_async("/bin/sh", &error) ) {
////				m_state = ProcessState::STARTING;
//			} else {
////				log_error() << "Can't start application : " << commandLine;
////				m_state = ProcessState::START_FAILED;
//			}

		});

} else {
	std::ostringstream oss;
	std::vector<std::string> envVarVec = {"MOUNT_DIR=" + m_mountDir};

	oss << "lxc-attach -n " << name() << " " << commandLine;

	std::string command = oss.str();

	log_debug() << command;

	pid_t pid = -1;
    auto executeCommandVec = Glib::shell_parse_argv(command);
	Glib::spawn_async_with_pipes(".", executeCommandVec, envVarVec,
			 Glib::SPAWN_SEARCH_PATH | Glib::SPAWN_LEAVE_DESCRIPTORS_OPEN,
			sigc::slot<void>(), &pid);

	log_debug() << "pid: " << pid;

	return pid;
}

}

void Container::stop()
{
	log_debug() << "Stopping the container";
	m_container->stop(m_container);
}

void Container::destroy()
{
	stop();
	if(isLXC_C_APIEnabled()) {

	log_debug() << " Shutting down container";
		kill(m_container->init_pid(m_container), SIGTERM);
		m_container->shutdown(m_container, 2);
} else {
    int maxCmdLen = sysconf(_SC_ARG_MAX);
    char lxcCommand[maxCmdLen];

    // Command to destroy container
    snprintf(lxcCommand, maxCmdLen, "lxc-destroy -f -n %s", name());

    std::string destroyCommand(lxcCommand);
    log_debug() << destroyCommand;
    Glib::spawn_command_line_sync(destroyCommand);
}
}


bool Container::bindMountDir(const std::string &src, const std::string &dst)
{
    int mountRes = mount(src.c_str(), // source
                         dst.c_str(), // target
                         "",          // fstype
                         MS_BIND,     // flags
                         NULL);       // data

    if (mountRes == 0) {
        // Success
        m_mounts.push_back(dst);
        log_verbose() << "Mounted folder " << src << " in " << dst;
    } else {
        // Failure
        log_warning("Could not mount dir into container: src=%s, dst=%s err=%s",
                  src.c_str(),
                  dst.c_str(),
                  strerror(errno));
    }

    return (mountRes == 0);
}

bool Container::setApplication(const std::string &appId)
{
	/* When we know which app that will be run we need to
	 * do some setup, like mount in the application bin and
	 * shared directories.
	 */

	// The directory(ies) to be mounted is known by convention, e.g.
    // /var/am/<appId>/bin/ and /var/am/<appId>/shared/

    // bind mount /var/am/<appId>/bin/ into /var/am/late_mounts/<contid>/bin
    // this directory will be accessible in container as according to
    // the lxc-pelagicontain template
    std::string appDirBase = m_containerRoot + "/" + appId;
    std::string dstDirBase = m_mountDir + "/" + m_name;

    bool allOk = true;
    allOk &= bindMountDir(appDirBase + "/bin", dstDirBase + "/bin");
    allOk &= bindMountDir(appDirBase + "/shared", dstDirBase + "/shared");
    allOk &= bindMountDir(appDirBase + "/home", dstDirBase + "/home");

    allOk = true;

    return allOk;
}
