#pragma once

#include <sys/stat.h>

#include <glibmm.h>
#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

//#include "pelagicore-common.h"

//#include "log.h"
#include "paminterface.h"
#include "pelagicontain.h"
#include "pelagicontaintodbusadapter.h"
//#include "dbusmainloop.h"

#include "systemcallinterface.h"

#include <fstream>

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

namespace pelagicontain {

class PelagicontainLib {

	LOG_DECLARE_CLASS_CONTEXT("PC", "Pelagicontain library")
	;

public:
	PelagicontainLib(Glib::RefPtr<Glib::MainContext> ml,
			const char* containerRootFolder = PELAGICONTAIN_DEFAULT_WORKSPACE
			, const char* cookie = "1"
			 , const char* configFilePath = PELAGICONTAIN_DEFAULT_CONFIG
			 );

	~PelagicontainLib() {
	}

	void shutdown() {
		pelagicontain.shutdown();
	}

	const Container& getContainer() const {
		return container;
	}

	Container& getContainer() {
		return container;
	}

	bool isFolder(const std::string& path) {
		// TODO: check if it is a folder
		std::ifstream f( path.c_str() );
		if ( f.good() ) {
			f.close();
			return true;
		} else {
			f.close();
			return false;
		}
	}

	bool isInitialized() {
		return m_initialized;
	}

	ReturnCode init() {

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

	Pelagicontain& getPelagicontain() {
		return pelagicontain;
	}

	ReturnCode registerDBusService() {

		/* The request_name call does not return anything but raises an
		 * exception if the name cannot be requested.
		 */
		std::string name = "com.pelagicore.Pelagicontain" + m_cookie;
		m_bus->request_name( name.c_str() );

		std::string objectPath = "/com/pelagicore/Pelagicontain";

		m_pcAdapter = std::unique_ptr<PelagicontainToDBusAdapter
					      > ( new PelagicontainToDBusAdapter(*m_bus, objectPath, pelagicontain) );

		return ReturnCode::SUCCESS;
	}

private:

	/**
	 * Check if the workspace is present and create it if needed
	 */
	ReturnCode checkWorkspace();

//	std::unique_ptr<DBus::Connection> m_bus;
	DBus::Connection* m_bus;  // we don't use a unique_ptr here because the destructor of that object causes a SEGFAULT... TODO : fix

	std::string containerName;
	std::string containerConfig;
	std::string containerRoot;
	std::string containerDir;
	std::string gatewayDir;
	std::string m_cookie;

	Container container;

	Glib::RefPtr<Glib::MainContext> m_ml;

	ControllerInterface controllerInterface;
	SystemcallInterface systemcallInterface;
	Pelagicontain pelagicontain;

    DBus::Glib::BusDispatcher dispatcher;

    std::unique_ptr<PelagicontainToDBusAdapter> m_pcAdapter;

	std::vector<std::unique_ptr<Gateway> > m_gateways;

	std::unique_ptr<PAMInterface> pamInterface;

	bool m_initialized = false;

};

}
