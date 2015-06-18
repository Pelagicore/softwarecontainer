#include "pelagicontain-lib.h"

#include "generators.h" /* used for gen_ct_name */

#ifdef ENABLE_PULSEGATEWAY
#include "pulsegateway.h"
#endif

#ifdef ENABLE_NETWORKGATEWAY
#include "networkgateway.h"
#endif

#ifdef ENABLE_DBUSGATEWAY
#include "dbusgateway.h"
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
#include "devicenodegateway.h"
#endif

#ifdef ENABLE_CGROUPSGATEWAY
#include "cgroupsgateway.h"
#endif

#include "envgateway.h"
#include "waylandgateway.h"
#include "filegateway.h"

#include "config.h"

namespace pelagicontain {

PelagicontainWorkspace &getDefaultWorkspace()
{
    static PelagicontainWorkspace defaultWorkspace;
    return defaultWorkspace;
}

PelagicontainLib::PelagicontainLib(PelagicontainWorkspace &workspace) :
    m_workspace(workspace),
    m_container( getContainerID()
               , m_containerName
               , m_workspace.m_containerConfig
               , m_workspace.m_containerRoot
               , m_workspace.m_containerShutdownTimeout)
{
    m_pelagicontain.setMainLoopContext(m_ml);
}

PelagicontainLib::~PelagicontainLib()
{
}

void PelagicontainLib::setContainerIDPrefix(const std::string &name)
{
    m_containerID = name + Generator::gen_ct_name();
    log_debug() << "Assigned container ID " << m_containerID;
}

void PelagicontainLib::setContainerName(const std::string &name)
{
    m_containerName = name;
    log_debug() << m_container.toString();
}

void PelagicontainLib::validateContainerID()
{
    if (m_containerID.size() == 0) {
        setContainerIDPrefix("PLC-");
    }
}

ReturnCode PelagicontainWorkspace::deleteWorkspace()
{
    if (isDirectory(m_containerRoot)) {
        rmdir(m_containerRoot.c_str());
    }

    return existsInFileSystem(m_containerRoot) ? ReturnCode::FAILURE : ReturnCode::SUCCESS;
}

//FileToolkitWithUndo PelagicontainLib::s_fileToolkit;

ReturnCode PelagicontainWorkspace::checkWorkspace()
{
    if (!isDirectory(m_containerRoot)) {

        createDirectory(m_containerRoot);
        std::string lateMountPath = m_containerRoot + LATE_MOUNT_PATH;
        createDirectory(lateMountPath);
        createSharedMountPoint(lateMountPath);

        std::string cmdLine = INSTALL_PREFIX;
        cmdLine += "/bin/setup_pelagicontain.sh " + m_containerRoot;
        log_debug() << "Creating workspace : " << cmdLine;
        int returnCode;
        try {
            Glib::spawn_sync("", Glib::shell_parse_argv(
                            cmdLine), static_cast<Glib::SpawnFlags>(0) /*value available as Glib::SPAWN_DEFAULT in recent glibmm*/,
                        sigc::slot<void>(), nullptr,
                        nullptr, &returnCode);
        } catch (Glib::SpawnError e) {
            log_error() << "Failed to spawn " << cmdLine << ": code " << e.code() << " msg: " << e.what();
            return ReturnCode::FAILURE;
        }
        if (returnCode != 0) {
            log_error() << "Return code of " << cmdLine << " is non-zero";
            return ReturnCode::FAILURE;
        }
    }

    return ReturnCode::SUCCESS;
}

ReturnCode PelagicontainLib::preload()
{

    if (isError(m_container.initialize())) {
        log_error() << "Could not setup container for preloading";
        return ReturnCode::FAILURE;
    }

    m_pcPid = m_pelagicontain.preload(m_container);

    if (m_pcPid != 0) {
        log_debug() << "Started container with PID " << m_pcPid;
    } else {
        // Fatal failure, only do necessary cleanup
        log_error() << "Could not start container, will shut down";
        return ReturnCode::FAILURE;
    }

    return ReturnCode::SUCCESS;
}

ReturnCode PelagicontainLib::init()
{
    validateContainerID();

    if (m_ml->gobj() == nullptr) {
        log_error() << "Main loop context must be set first !";
        return ReturnCode::FAILURE;
    }

    if (m_pelagicontain.getContainerState() != ContainerState::PRELOADED) {
        if (isError(preload())) {
            log_error() << "Failed to preload container";
            return ReturnCode::FAILURE;
        }
    }

#ifdef ENABLE_NETWORKGATEWAY
    m_gateways.push_back(std::unique_ptr<Gateway>(new NetworkGateway()));
#endif

#ifdef ENABLE_PULSEGATEWAY
    m_gateways.push_back(std::unique_ptr<Gateway>(new PulseGateway(getGatewayDir(), getContainerID())));
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
    m_gateways.push_back(std::unique_ptr<Gateway>(new DeviceNodeGateway()));
#endif

#ifdef ENABLE_DBUSGATEWAY
    m_gateways.push_back(std::unique_ptr<Gateway>(new DBusGateway(
                        DBusGateway::SessionProxy,
                        getGatewayDir(),
                        getContainerID())));

    m_gateways.push_back(std::unique_ptr<Gateway>(new DBusGateway(
                        DBusGateway::SystemProxy,
                        getGatewayDir(),
                        getContainerID())));
#endif

#ifdef ENABLE_CGROUPSGATEWAY
    m_gateways.push_back(std::unique_ptr<Gateway>(new CgroupsGateway()));
#endif

    m_gateways.push_back(std::unique_ptr<Gateway>(new WaylandGateway()));
    m_gateways.push_back(std::unique_ptr<Gateway>(new FileGateway()));
    m_gateways.push_back(std::unique_ptr<Gateway>(new EnvironmentGateway()));

    for (auto &gateway : m_gateways) {
        m_pelagicontain.addGateway(*gateway);
    }

    // The pid might not valid if there was an error spawning. We should only
    // connect the watcher if the spawning went well.
    if (m_pcPid != 0) {
        addProcessListener(m_connections, m_pcPid, [&] (pid_t pid, int exitCode) {
                        m_pelagicontain.shutdownContainer();
                    }, m_ml);
    }

    m_initialized = true;

    return ReturnCode::SUCCESS;
}

void PelagicontainLib::openTerminal(const std::string &terminalCommand) const
{
    std::string command = logging::StringBuilder() << terminalCommand << " lxc-attach -n " << m_container.id();
    log_info() << command;
    system(command.c_str());
}

}
