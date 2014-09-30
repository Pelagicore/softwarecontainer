/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PELAGICONTAIN_H
#define PELAGICONTAIN_H

#include <sys/types.h>
#include <glibmm.h>

#include "container.h"
#include "paminterface.h"

class Gateway;

enum class ContainerState {
	CREATED,
	PRELOADED,
	READY,
	RUNNING,
	TERMINATED
};

class ContainerListener {
public:
	virtual void onContainerStateChanged(ContainerState state) = 0;
};

class Pelagicontain {

    LOG_DECLARE_CLASS_CONTEXT("PCON", "Pelagicontain");

public:
    /*! Constructor
     *
     * \param cookie A unique identifier used to distinguish unique instances
     *  of Pelagicontain
     */
    Pelagicontain(const std::string &cookie);

    ~Pelagicontain();

    void setPAM(PAMAbstractInterface& pamInterface) {
    	m_pamInterface = &pamInterface;
    }

    /*! Add a gateway.
     *
     * \param gateway A gateway that will be used by the container
     */
    void addGateway(Gateway & gateway);

    /*! Starts the container preloading phase.
     *
     * The 'preload' phase is initiated by a call to this method. If an invalid
     * pid is returned, there has been an error and the container will not
     * be functional. This is considered fatal, no further calls should be
     * made and we should shut down.
     *
     * \param container The container object to preload
     *
     * \return The PID of the container, '0' on error
     */
    pid_t preload(Container &container);

    /*! Initiates the 'launch' phase.
     *
     * Registers Pelagicontain with Platform Access Manager and awaits the
     * gateway configurations. This is the initial stage of the launch phase.
     *
     * \param appId An application identifier used to fetch what capabilities
     *  the application wants access to
     */
    void launch(const std::string &appId);


    void setApplicationID(const std::string &appId);


    pid_t launchCommand(const std::string &commandLine);

    /*! Continues the 'launch' phase by allowing gateway configurations to
     *  be set.
     *
     * Platform Access Manager calls this method after Pelagicontain has
     * registered as a client, and passes all gateway configurations as
     * argument. Pelagicontain sets the gateway configurations and activates
     * all gateways as a result of this call. The contained application
     * is then started.
     *
     * \param configs A map of gateway IDs and their respective configurations
     */
    void update(const GatewayConfiguration &configs);

    /**
     * Set the gateway configuration and activate them
     */
    void setGatewayConfigs(const GatewayConfiguration &configs);

    /*! Initiates the 'shutdown' phase.
     *
     * Shuts down the contained application, all gateways, then unregisters
     * as a client with Platform Access Manager, and finaly shuts down
     * the Pelagicontain component.
     */
    void shutdown();

    /*! Sets environment variable inside the container.
     *
     * Notifies the controller to set the environment variable passed as
     * argument inside the container.
     *
     * \param var The name of the environment variable to be set
     * \param val The value that the environement variable should be set to
     */
    void setContainerEnvironmentVariable(const std::string &var,
                                         const std::string &val);

    void shutdownContainer();

    ObservableProperty<ContainerState>& getContainerState() {
    	return m_containerState;
    }

    void setMainLoopContext(Glib::RefPtr<Glib::MainContext>& mainLoopContext) {
    	m_mainLoopContext = &mainLoopContext;
    }

private:
    void shutdownGateways();

    /*! Handle shutdown of the controller process inside a container
     *
     * Called by a SignalChildWatch source when the pid belonging to the
     * controller exits. Takes care of:
     *  - Cleaning up LXC
     *  - Issuing Pelagicontain::shutdownGateways()
     *  - PAMAbstractInterface::unregisterClient()
     *  - Killing the main event loop (and thus exiting Pelagicontain)
     *
     * \param pid pid of the controller as spawned by Pelagicontain::preload()
     * \param exitCode exit code of \a pid
     *
     */
    void onContainerShutdown(int pid, int exitCode);

    Container *m_container = nullptr;
    PAMAbstractInterface *m_pamInterface = nullptr;
    std::vector<Gateway *> m_gateways;
    std::string m_appId;
    const std::string& m_cookie;

    ObservableWritableProperty<ContainerState> m_containerState;

    // Keeps track of if someone has called launch
    bool m_launching = false;

    SignalConnectionsHandler m_connections;

	Glib::RefPtr<Glib::MainContext>* m_mainLoopContext = nullptr;

};

#endif /* PELAGICONTAIN_H */
