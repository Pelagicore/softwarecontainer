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

#include "dltgateway.h"
#include "waylandgateway.h"


PelagicontainLib::PelagicontainLib(const char* containerRootFolder, const char* configFilePath
		 ) : containerName(Generator::gen_ct_name()),
	containerConfig(configFilePath), containerRoot(containerRootFolder),
	containerDir(containerRoot + "/" + containerName), gatewayDir(containerDir + "/gateways"),
	container(containerName, containerConfig,
		  containerRoot),
	pelagicontain(m_cookie) {
	pelagicontain.setMainLoopContext(m_ml);
}

ReturnCode PelagicontainLib::checkWorkspace() {

	if ( !isDirectory(containerRoot) ) {
		std::string cmdLine = INSTALL_PREFIX;
		cmdLine += "/bin/setup_pelagicontain.sh " + containerRoot;
		log_debug() << "Creating workspace : " << cmdLine;
		int returnCode;
		Glib::spawn_sync("", Glib::shell_parse_argv(cmdLine), static_cast<Glib::SpawnFlags>(0) /*value available as Glib::SPAWN_DEFAULT in recent glibmm*/, sigc::slot<void>(), nullptr,
				 nullptr, &returnCode);
		if (returnCode != 0)
			return ReturnCode::FAILURE;
	}

	return ReturnCode::SUCCESS;
}

ReturnCode PelagicontainLib::preload() {

	// Make sure path ends in '/' since it might not always be checked
	if (containerRoot.back() != '/') {
		containerRoot += "/";
	}

	if ( isError( checkWorkspace() ) )
		return ReturnCode::FAILURE;

	if ( isError( container.initialize() ) ) {
		log_error() << "Could not setup container for preloading";
		return ReturnCode::FAILURE;
	}

	m_pcPid = pelagicontain.preload(container);

	if (m_pcPid != 0) {
		log_debug() << "Started container with PID " << m_pcPid;
		sleep(1);
	} else {
		// Fatal failure, only do necessary cleanup
		log_error() << "Could not start container, will shut down";
	}

	return ReturnCode::SUCCESS;
}

ReturnCode PelagicontainLib::init(bool bRegisterDBusInterface) {

	if (m_ml->gobj() == nullptr) {
		log_error() << "Main loop context must be set first !";
		return ReturnCode::FAILURE;
	}

	if (pelagicontain.getContainerState() != ContainerState::PRELOADED)
		if (isError(preload()))
			return ReturnCode::FAILURE;

	if (bRegisterDBusInterface)
		if (m_cookie.size()==0)
			m_cookie = containerName;

	DBus::default_dispatcher = &dispatcher;

	m_bus = new DBus::Connection(DBus::Connection::SessionBus());
	dispatcher.attach(m_ml->gobj());
	pamInterface = std::unique_ptr<PAMInterface>(new PAMInterface(*m_bus));

	if (bRegisterDBusInterface)
		registerDBusService();

	pelagicontain.setPAM(*pamInterface.get());

#ifdef ENABLE_NETWORKGATEWAY
	m_gateways.push_back( std::unique_ptr<Gateway>( new NetworkGateway(systemcallInterface) ) );
#endif

#ifdef ENABLE_PULSEGATEWAY
	m_gateways.push_back( std::unique_ptr<Gateway>( new PulseGateway(gatewayDir, containerName) ) );
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
	m_gateways.push_back( std::unique_ptr<Gateway>( new DeviceNodeGateway() ) );
#endif

#ifdef ENABLE_DBUSGATEWAY
	m_gateways.push_back( std::unique_ptr<Gateway>( new DBusGateway(
									systemcallInterface,
									DBusGateway::SessionProxy,
									gatewayDir,
									containerName) ) );

	m_gateways.push_back( std::unique_ptr<Gateway>( new DBusGateway(
									systemcallInterface,
									DBusGateway::SystemProxy,
									gatewayDir,
									containerName) ) );
#endif

	m_gateways.push_back( std::unique_ptr<Gateway>( new DLTGateway() ) );

	m_gateways.push_back( std::unique_ptr<Gateway>( new WaylandGateway() ) );

	for (auto& gateway : m_gateways)
		pelagicontain.addGateway(*gateway);

    // The pid might not valid if there was an error spawning. We should only
    // connect the watcher if the spawning went well.
    if (m_pcPid != 0) {
    	addProcessListener(m_connections, m_pcPid, [&] (pid_t pid, int exitCode) {
    		pelagicontain.shutdownContainer();
    	}, m_ml);
    }

	if (bRegisterDBusInterface) {
		/* If we can't communicate with PAM then there is nothing we can
		 * do really, better to just exit.
		 */
		bool pamRunning = m_bus->has_name("com.pelagicore.PAM");
		if (!pamRunning) {
			log_error() << "PAM not running, exiting";
			return ReturnCode::FAILURE;
		}
	}

	m_initialized = true;

	return ReturnCode::SUCCESS;
}
