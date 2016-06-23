#include "libpelagicontain.h"
#include "generators.h" /* used for gen_ct_name */

#ifdef ENABLE_PULSEGATEWAY
#include "gateway/pulsegateway.h"
#endif

#ifdef ENABLE_NETWORKGATEWAY
#include "gateway/networkgateway.h"
#endif

#ifdef ENABLE_DBUSGATEWAY
#include "gateway/dbusgateway.h"
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
#include "gateway/devicenodegateway.h"
#endif

#ifdef ENABLE_CGROUPSGATEWAY
#include "gateway/cgroupsgateway.h"
#endif

#include "gateway/envgateway.h"
#include "gateway/waylandgateway.h"
#include "gateway/filegateway.h"

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
    shutdown();
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
        if(isError(createDirectory(m_containerRoot))) {
            return ReturnCode::FAILURE;
        }
    }
    
    // TODO: Have a way to check for the bridge using C/C++ instead of a
    // shell script. Libbridge and/or netfilter?
    std::string cmdLine = INSTALL_PREFIX;
    cmdLine += "/bin/setup_pelagicontain.sh";
    log_debug() << "Creating workspace : " << cmdLine;
    int returnCode;
    try {
        Glib::spawn_sync("", Glib::shell_parse_argv(cmdLine),
                         static_cast<Glib::SpawnFlags>(0), // Glib::SPAWN_DEFAULT in newer versions of glibmm
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

    // TODO: When this is used together with spawning using lxc.init, we get
    //       glib errors about ECHILD
    // The pid might not valid if there was an error spawning. We should only
    // connect the watcher if the spawning went well.
    if (m_pcPid != 0) {
        addProcessListener(m_connections, m_pcPid, [&] (pid_t pid, int exitCode) {
                        m_pelagicontain.shutdownContainer();
                    }, m_ml);
    } else {
        log_error() << "Pelagicontain pid is 0, this is an error!";
        return ReturnCode::FAILURE;
    }
    m_initialized = true;
    return ReturnCode::SUCCESS;
}

void PelagicontainLib::openTerminal(const std::string &terminalCommand) const
{
    std::string command = logging::StringBuilder() << "lxc-attach -n " << m_container.id() << " " << terminalCommand;
    log_info() << command;
    system(command.c_str());
}

}
