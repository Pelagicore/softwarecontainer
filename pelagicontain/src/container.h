/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTAINER_H
#define CONTAINER_H

#include <string>
#include <vector>

#include "gateway.h"
#include "log.h"
#include "pelagicontain-common.h"

/*! Container is an abstraction of the specific containment technology used.
 *
 * The Container class is meant to be an abstraction of the containment
 * technology, so that Pelagicontain can view it as a generic concept that
 * implements the specifics behind the conceptual phases of 'Pereload', 'Launch',
 * and 'Shutdown'.
 */
class Container : public ControllerAbstractInterface
{
    LOG_DECLARE_CLASS_CONTEXT("CONT", "Container");

public:

    /// A function to be executed in the container
    typedef std::function<void()> ContainerFunction;

    typedef std::function<void(pid_t pid, int returnCode)> ProcessListenerFunction;

    /*!
     * Constructor
     *
     * \param name Name of the container
     * \param configFile Path to the configuration file (including the file name)
     * \param containerRoot A path to the root of the container, i.e. the base
     *  path to e.g. the configurations and application root
     * \param containedCommand The command to be executed inside the container
     */
    Container(const std::string &name,
              const std::string &configFile,
              const std::string &containerRoot);

    ~Container();

    /*!
     * Calls the lxc-create command.
     */
    void create();

    /*!
     * Start the container
     *
     * \return The pid of the init process of the container
     */
    pid_t start();

    pid_t attach(const std::string& commandLine, ProcessListenerFunction listener, const EnvironmentVariables& variables);
    pid_t attach(const std::string& commandLine, ProcessListenerFunction listener);

    pid_t executeInContainer(ContainerFunction function, ProcessListenerFunction listener, const EnvironmentVariables& variables);

    /*!
     * Calls the lxc-destroy command.
     */
    void destroy();

    void stop();

    /*!
     * Setup the container for a specific app
     *
     * Setup the container so that the app specific directories are available
     * inside the container. If any of these directories could not setup \c false
     * is returned which is considered fatal, the app should not be started if
     * this is the case. Method returns \c true if all directories could be
     * made available
     *
     * \param appId A string with the application ID
     *
     * \return true or false
     */
    bool setApplication(const std::string &appId);

    /*!
     * Setup the container for preloading
     *
     * Setup the container so directories are available for later use when
     * setApplication is called. If the 'late_mount' directory is missing
     * or any subddirectory to it could not be created, \c false is returned,
     * if all went well, \c true is returned.
     *
     * \return true or false
     */
    ReturnCode initialize();

    std::string toString();

    const char *name() const;

    std::string containerDir() const {
    	return m_containerRoot + name();
    }

    ReturnCode setEnvironmentVariable(const std::string& var, const std::string& val) override {
    	log_error() << "Setting env variable in container " << var << "=" << val;
    	m_env[var] = val;
    	return ReturnCode::SUCCESS;
    }

    ReturnCode systemCall(const std::string &cmd) override {
    	executeInContainer([cmd=cmd]() {
        	log_error() << "Executing system command in container : " << cmd;
    		system(cmd.c_str());
    	}, [cmd=cmd](pid_t pid , int returnCode) {
        	log_error() << "Command finished: " << cmd;
    	}, m_env);
    	sleep(1);
    	return ReturnCode::SUCCESS;
    }

private:

    static int executeInContainerEntryFunction(void* param);

    /*
     * Check if path is a directory
     */
    bool isDirectory(const std::string &path);

    /*
     * Create a directory, and if successful append it to a list of dirs
     * to be deleted in the dtor. Since nestled dirs will need to be
     * deleted in reverse order to creation insert to the beginning of
     * the list.
     */
    ReturnCode createDirectory(const std::string &path);

    /*
     * Create a bind mount. On success the mount will be added to a list of
     * mounts that will be unmounted in the dtor.
     */
    bool bindMountDir(const std::string &src, const std::string &dst);

    /*
     * List of all, by the container, mounted directories. These directories
     * should be unmounted in the destructor.
     */
    std::vector<std::string> m_mounts;

    /*
     * List of all, by the container, created directories. These directories
     * should be deleted in the destructor.
     */
    std::vector<std::string> m_dirs;

    /*
     * The LXC configuration file for this container
     */
    std::string m_configFile;

    /*
     * The unique name of the LXC container
     */
    std::string m_name;

    struct lxc_container *m_container = nullptr;

    std::string m_containerRoot;
    std::string m_mountDir;

    std::vector<const char*> m_LXCContainerStates;

    EnvironmentVariables m_env;

};

#endif //CONTAINER_H
