/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <glibmm.h>

#include "abstractcontroller.h"
#include "pelagicontain-common.h"

class Controller:
    public AbstractController
{
	LOG_DECLARE_CLASS_CONTEXT("CTRL", "Controller");

	static constexpr const char* LD_LIBRARY_PATH_ENV_VARIABLE = "LD_LIBRARY_PATH";
	static constexpr const char* PATH_ENV_VARIABLE = "PATH";

public:
    Controller(Glib::RefPtr<Glib::MainLoop> ml);
    ~Controller();

<<<<<<< df8ab6bef560ba4315c245778c1e8adaffa5313a
    virtual int runApp();
    /*!  
     *  Kill an app by sending SIGINT to it. If it has not shut down within
     *  five seconds, a SIGKILL is sent to it. 
     */
    virtual void killApp();
    virtual void setEnvironmentVariable(const std::string &variable,
                                        const std::string &value);
    virtual void systemCall(const std::string &command);
=======
    pid_t runApp() override;
    void killApp() override;
    void setEnvironmentVariable(const std::string &variable,
                                        const std::string &value) override;
    void systemCall(const std::string &command) override;
>>>>>>> Cleanup

private:

    void appendToEnvVariable(const char* variableName, const char* value) {
    	auto ldLibraryPathEnv = getenv(variableName);
    	std::string ldLibraryPath;
    	if (ldLibraryPathEnv != nullptr)
    		ldLibraryPath = ldLibraryPathEnv;

    	ldLibraryPath += value;
    	setenv(variableName, ldLibraryPath.c_str(), true);
    	log_info() << "Env variable " << variableName << " set to " << ldLibraryPath;
    }

    void adjustEnvironment();
    void shutdown();
    bool killMainLoop();
    void childSetupSlot();
    void handleAppShutdownSlot(int pid, int exitCode);

    Glib::RefPtr<Glib::MainLoop> m_ml;
    pid_t m_pid;
};

#endif //CONTROLLER_H
