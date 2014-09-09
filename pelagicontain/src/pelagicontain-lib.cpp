#include "pelagicontain-lib.h"

#include "generators.h" /* used for gen_ct_name */

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

	if ( !isFolder(containerRoot) ) {
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
