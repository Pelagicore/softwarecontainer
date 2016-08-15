#include "libsoftwarecontainer.h"
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

namespace softwarecontainer {

SoftwareContainerWorkspace &getDefaultWorkspace()
{
    static SoftwareContainerWorkspace defaultWorkspace;
    return defaultWorkspace;
}

SoftwareContainerLib::SoftwareContainerLib(SoftwareContainerWorkspace &workspace) :
    m_workspace(workspace),
    m_container( getContainerID()
               , m_containerName
               , m_workspace.m_containerConfig
               , m_workspace.m_containerRoot
               , m_workspace.m_containerShutdownTimeout)
{
    m_containerState = ContainerState::CREATED;
}

SoftwareContainerLib::~SoftwareContainerLib()
{
}

void SoftwareContainerLib::setMainLoopContext(Glib::RefPtr<Glib::MainContext> mainLoopContext)
{
    m_mainLoopContext = mainLoopContext;
}


void SoftwareContainerLib::setContainerIDPrefix(const std::string &name)
{
    m_containerID = name + Generator::gen_ct_name();
    log_debug() << "Assigned container ID " << m_containerID;
}

void SoftwareContainerLib::setContainerName(const std::string &name)
{
    m_containerName = name;
    log_debug() << m_container.toString();
}

void SoftwareContainerLib::validateContainerID()
{
    if (m_containerID.size() == 0) {
        setContainerIDPrefix("PLC-");
    }
}

ReturnCode SoftwareContainerWorkspace::checkWorkspace()
{
    if (!isDirectory(m_containerRoot)) {
        log_debug() << "Container root " << m_containerRoot << " does not exist, trying to create";
        if(isError(createDirectory(m_containerRoot))) {
            log_debug() << "Failed to create container root directory";
            return ReturnCode::FAILURE;
        }
    }
    
#ifdef ENABLE_NETWORKGATEWAY
    // TODO: Have a way to check for the bridge using C/C++ instead of a
    // shell script. Libbridge and/or netfilter?
    std::string cmdLine = INSTALL_PREFIX;
    cmdLine += "/bin/setup_softwarecontainer.sh";
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
#endif

    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainerLib::preload()
{
    log_debug() << "Initializing container";
    if (isError(m_container.initialize())) {
        log_error() << "Could not setup container for preloading";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Creating container";
    if (isError(m_container.create())) {
        return ReturnCode::FAILURE;
    }

    log_debug() << "Starting container";
    m_pcPid = m_container.start();
    if (m_pcPid == INVALID_PID) {
        log_error() << "Could not start the container during preload";
        return ReturnCode::FAILURE;
    }

    log_debug() << "Started container with PID " << m_pcPid;
    m_containerState.setValueNotify(ContainerState::PRELOADED);
    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainerLib::init()
{
    validateContainerID();

    if (m_mainLoopContext->gobj() == nullptr) {
        log_error() << "Main loop context must be set first !";
        return ReturnCode::FAILURE;
    }

    if (getContainerState() != ContainerState::PRELOADED) {
        if (isError(preload())) {
            log_error() << "Failed to preload container";
            return ReturnCode::FAILURE;
        }
    }

#ifdef ENABLE_NETWORKGATEWAY
    addGateway(new NetworkGateway());
#endif

#ifdef ENABLE_PULSEGATEWAY
    addGateway(new PulseGateway());
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
    addGateway(new DeviceNodeGateway());
#endif

#ifdef ENABLE_DBUSGATEWAY
    addGateway(new DBusGateway( DBusGateway::SessionProxy, getGatewayDir(), getContainerID()));
    addGateway(new DBusGateway( DBusGateway::SystemProxy, getGatewayDir(), getContainerID()));
#endif

#ifdef ENABLE_CGROUPSGATEWAY
    addGateway(new CgroupsGateway());
#endif

    addGateway(new WaylandGateway());
    addGateway(new FileGateway());
    addGateway(new EnvironmentGateway());

    // TODO: When this is used together with spawning using lxc.init, we get
    //       glib errors about ECHILD
    // TODO: The pid might not valid if there was an error spawning. We should only
    //       connect the watcher if the spawning went well.
    if (m_pcPid != INVALID_PID) {
        addProcessListener(m_connections, m_pcPid, [&] (pid_t pid, int exitCode) {
            shutdown(m_workspace.m_containerShutdownTimeout);
        }, m_mainLoopContext);
    } else {
        log_error() << "SoftwareContainer pid is " << INVALID_PID << ", this is an error!";
        return ReturnCode::FAILURE;
    }

    m_initialized = true;
    return ReturnCode::SUCCESS;
}

void SoftwareContainerLib::addGateway(Gateway *gateway)
{
    gateway->setContainer(m_container);
    m_gateways.push_back(std::unique_ptr<Gateway>(gateway));
}

void SoftwareContainerLib::openTerminal(const std::string &terminalCommand) const
{
    std::string command = logging::StringBuilder() << "lxc-attach -n " << m_container.id() << " " << terminalCommand;
    log_info() << command;
    system(command.c_str());
}


pid_t SoftwareContainerLib::launchCommand(const std::string &commandLine)
{
    /*
    if (m_mainLoopContext == nullptr) {
        log_error() << "Main loop context needs to be set before calling launchCommand";
        return INVALID_PID;
    }
    */

    log_debug() << "launchCommand called with commandLine: " << commandLine;
    pid_t pid = m_container.attach(commandLine);
    if (pid == INVALID_PID) {
        log_error() << "Attach returned invalid pid, launchCommand fails";
        return INVALID_PID;
    }

    // TODO: Why do we shutdown as soon as one process exits?
    addProcessListener(m_connections, pid, [&](pid_t pid, int returnCode) {
        shutdown();
    }, m_mainLoopContext);

    return pid;
}

void SoftwareContainerLib::updateGatewayConfiguration(const GatewayConfiguration &configs)
{
    log_debug() << "updateGatewayConfiguration called" << configs;
    setGatewayConfigs(configs);
}

void SoftwareContainerLib::setGatewayConfigs(const GatewayConfiguration &configs)
{
    // Go through the received configs and see if they match any of
    // the running gateways, if so: set their respective config

    for (auto &gateway : m_gateways) {
        std::string gatewayId = gateway->id();

        if (configs.count(gatewayId) != 0) {
            std::string config = configs.at(gatewayId);
            gateway->setConfig(config);
        }
    }

    for (auto &gateway : m_gateways) {
        if (gateway->isConfigured()) {
            gateway->activate();
        }
    }

    m_containerState.setValueNotify(ContainerState::READY);

}

ReturnCode SoftwareContainerLib::shutdown()
{
    return shutdown(m_workspace.m_containerShutdownTimeout);
}

ReturnCode SoftwareContainerLib::shutdown(unsigned int timeout)
{
    log_debug() << "shutdown called"; // << logging::getStackTrace();
    if(isError(shutdownGateways())) {
        log_error() << "Could not shut down all gateways cleanly, check the log";
    }

    if(isError(m_container.destroy(timeout))) {
        log_error() << "Could not destroy the container during shutdown";
        return ReturnCode::FAILURE;
    }

    m_containerState.setValueNotify(ContainerState::TERMINATED);
    return ReturnCode::SUCCESS;
}

ReturnCode SoftwareContainerLib::shutdownGateways()
{
    ReturnCode status = ReturnCode::SUCCESS;
    for (auto &gateway : m_gateways) {
        if (gateway->isActivated()) {
            if (!gateway->teardown()) {
                log_warning() << "Could not tear down gateway cleanly: " << gateway->id();
                status = ReturnCode::FAILURE;
            }
        }
    }

    m_gateways.clear();
    return status;
}

bool SoftwareContainerLib::isInitialized() const
{
    return m_initialized;
}

Container &SoftwareContainerLib::getContainer()
{
    return m_container;
}

std::string SoftwareContainerLib::getContainerDir()
{
    return m_workspace.m_containerRoot + "/" + getContainerID();
}

std::string SoftwareContainerLib::getGatewayDir()
{
    return getContainerDir() + "/gateways";
}

const std::string &SoftwareContainerLib::getContainerID()
{
    return m_containerID;
}

ObservableProperty<ContainerState> &SoftwareContainerLib::getContainerState()
{
    return m_containerState;
}

}
