#pragma once

#include <sys/stat.h>
#include <sys/wait.h>

#include <glibmm.h>
#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

#include "paminterface.h"
#include "pelagicontain.h"
#include "pelagicontaintodbusadapter.h"

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

	LOG_DECLARE_CLASS_CONTEXT("PC", "Pelagicontain library");

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

	ReturnCode init(bool bRegisterDBusInterface = false) {

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

	Pelagicontain& getPelagicontain() {
		return pelagicontain;
	}

private:

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

/**
 * Use this class to execute a command in a container
 */
class CommandJob {

public:

	static constexpr int UNASSIGNED_STREAM = -1;

	CommandJob(PelagicontainLib& lib, const std::string& command) :
			m_lib(lib) {
		m_command = command;
	}

	ReturnCode start( ) {
		m_pid = m_lib.getContainer().attach(m_command, m_env, m_stdin[0], m_stdout[1], m_stderr[1]);
		return (m_pid != 0) ? ReturnCode::SUCCESS : ReturnCode::FAILURE;
	}

	void setEnvironnmentVariable(const std::string& key, const std::string& value) {
		m_env[key] = value;
	}

	void captureStdin() {
		pipe(m_stdin);
	}

	void captureStdout() {
		pipe(m_stdout);
	}

	void captureStderr() {
		pipe(m_stderr);
	}

	int wait() {
		int status;
		waitpid(m_pid, &status, 0);
		return status;
//		return WEXITSTATUS(status);
	}

	int stdout() {
		return m_stdout[0];
	}

	int stderr() {
		return m_stderr[0];
	}

	int stdin() {
		return m_stdin[1];
	}

	pid_t pid() {
		return m_pid;
	}

	/**
	 * That method always returns true as soon as the start() method has been called, even if the command fails to start,
	 * since we don't know if the exec() occuring after the fork into the container actually succeeds...
	 */
	bool isRunning() {
		// TODO : find a way to test whether the exec() which occurs in the container succeeded
		return (m_pid!=0);
	}

	std::string toString() const {
		return logging::StringBuilder() << "Pelagicontain job. command: " << m_command << " stdin:" << m_stdin[0]
				<< " stdout:" << m_stdout[1];
	}

private:
	std::string m_command;
	EnvironmentVariables m_env;
	PelagicontainLib& m_lib;
	pid_t m_pid = 0;
	int m_stdin[2] = {UNASSIGNED_STREAM, UNASSIGNED_STREAM};
	int m_stdout[2] = {UNASSIGNED_STREAM, UNASSIGNED_STREAM};
	int m_stderr[2] = {UNASSIGNED_STREAM, UNASSIGNED_STREAM};
};

}
