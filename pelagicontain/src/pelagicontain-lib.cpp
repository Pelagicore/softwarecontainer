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

PelagicontainLib::PelagicontainLib(Glib::RefPtr<Glib::MainContext> ml, const char* containerRootFolder, const char* cookie
		, const char* configFilePath
		 ) : containerName(Generator::gen_ct_name()),
	containerConfig(configFilePath), containerRoot(containerRootFolder),
	containerDir(containerRoot + "/" + containerName), gatewayDir(containerDir + "/gateways"), m_cookie(cookie),
	container(containerName, containerConfig,
		  containerRoot), controllerInterface(gatewayDir),
	pelagicontain(&controllerInterface, cookie) {
	m_ml = ml;
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

ReturnCode PelagicontainLib::init(bool bRegisterDBusInterface) {

	DBus::default_dispatcher = &dispatcher;

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

	m_bus = new DBus::Connection(DBus::Connection::SessionBus());
	dispatcher.attach(m_ml->gobj());

	pamInterface = std::unique_ptr<PAMInterface>(new PAMInterface(*m_bus));

	if (bRegisterDBusInterface)
		registerDBusService();

	pelagicontain.setPAM(*pamInterface.get());

	/* If we can't communicate with PAM then there is nothing we can
	 * do really, better to just exit.
	 */
	bool pamRunning = m_bus->has_name("com.pelagicore.PAM");
	if (!pamRunning) {
		log_error() << "PAM not running, exiting";
		return ReturnCode::FAILURE;
	}

#ifdef ENABLE_NETWORKGATEWAY
	m_gateways.push_back( std::unique_ptr<Gateway>( new NetworkGateway(container, systemcallInterface) ) );
#endif

#ifdef ENABLE_PULSEGATEWAY
	m_gateways.push_back( std::unique_ptr<Gateway>( new PulseGateway(gatewayDir, containerName, container) ) );
#endif

#ifdef ENABLE_DEVICENODEGATEWAY
	m_gateways.push_back( std::unique_ptr<Gateway>( new DeviceNodeGateway(container) ) );
#endif

#ifdef ENABLE_DBUSGATEWAY
	m_gateways.push_back( std::unique_ptr<Gateway>( new DBusGateway(container,
									systemcallInterface,
									DBusGateway::SessionProxy,
									gatewayDir,
									containerName) ) );

	m_gateways.push_back( std::unique_ptr<Gateway>( new DBusGateway(container,
									systemcallInterface,
									DBusGateway::SystemProxy,
									gatewayDir,
									containerName) ) );
#endif

	for (auto& gateway : m_gateways)
		pelagicontain.addGateway(*gateway);

	pid_t pcPid = pelagicontain.preload(&container);

	if (!pcPid) {
		// Fatal failure, only do necessary cleanup
		log_error() << "Could not start container, will shut down";
	} else {
		log_debug() << "Started container with PID " << pcPid;
		// setup IPC between Pelagicontain and Controller
		if ( isError( pelagicontain.establishConnection() ) )
			return ReturnCode::FAILURE;
	}

	m_initialized = true;

	return ReturnCode::SUCCESS;
}
