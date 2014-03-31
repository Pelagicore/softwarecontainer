/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef PELAGICONTAIN_H
#define PELAGICONTAIN_H

#include <sys/types.h>

#include "container.h"
#include "paminterface.h"
#include "controllerinterface.h"
#include "mainloopabstractinterface.h"

class Pelagicontain {
public:
    /*! Constructor
     *
     * \param pamInterface A pointer to the Platform Access Manager interface
     * \param mainloopInterface A pointer to the mainloop interface
     * \param cookie A unique identifier used to distinguish unique instances
     * 	of Pelagicontain
     */
    Pelagicontain(PAMAbstractInterface *pamInterface,
                  MainloopAbstractInterface *mainloopInterface,
                  ControllerAbstractInterface *controllerInterface,
                  const std::string &cookie);

    ~Pelagicontain();

    /*! Add a gateway.
     *
     * \param gateway A gateway that will be used by the container
     */
    void addGateway(Gateway *gateway);

    /*! Starts the container preloading phase.
     *
     * The three phases, 'preload', 'launch', and 'shutdown' are initiated
     * by calling this method. The preload phase is started directly as
     * a result of calling this method. The launch and shotdown phases are
     * initiated by the client calling Pelagicontain::launch and
     * Pelagicontain::shutdown respectively as separate events, but those
     * must be preceeded by a call to this method. The call to this method
     * should be done as part of starting the whole Pelagicontain component.
     *
     * \param containerName Name of the container
     * \param containerConfig Path to the global config (/etc/pelagicontain commonly)
     * \param containerRoot A path to the root of the container, i.e. the base
     * 	path to e.g. the configurations and application root
     * \param containedCommand The command to be executed inside the container
     *
     * \return The PID of the container
     */
    pid_t preload(const std::string &containerName,
                  const std::string &containerConfig,
                  const std::string &containerRoot,
                  const std::string &containedCommand);

    /*! Initiates the 'launch' phase.
     *
     * Registers Pelagicontain with Platform Access Manager and awaits the
     * gateway configurations. This is the initial stage of the launch phase.
     *
     * \param appId An application identifier used to fetch what capabilities
     * 	the application wants access to
     */
    void launch(const std::string &appId);

    /*! Continues the 'launch' phase by allowing gateway configurations to
     * 	be set.
     *
     * Platform Access Manager calls this method after Pelagicontain has
     * registered as a client, and passes all gateway configurations as
     * argument. Pelagicontain sets the gateway configurations and activates
     * all gateways as a result of this call. The contained application
     * is then started.
     *
     * \param configs A map of gateway IDs and their respective configurations
     */
    void update(const std::map<std::string, std::string> &configs);

    /*! Initiates the 'shutdown' phase.
     *
     * Shuts down the contained application, all gateways, then unregisters
     * as a client with Platform Access Manager, and finaly shuts down
     * the Pelagicontain component.
     */
    void shutdown();

private:
    void setGatewayConfigs(const std::map<std::string, std::string> &configs);
    void activateGateways();
    void shutdownGateways();

    Container *m_container;
    PAMAbstractInterface *m_pamInterface;
    MainloopAbstractInterface *m_mainloopInterface;
    ControllerAbstractInterface *m_controllerInterface;
    std::vector<Gateway *> m_gateways;
    std::string m_appId;
    std::string m_cookie;
};

#endif /* PELAGICONTAIN_H */
