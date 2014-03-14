/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef NETWORKGATEWAY_H
#define NETWORKGATEWAY_H

#include "gateway.h"
#include "controllerinterface.h"

class NetworkGateway : public Gateway
{
public:
	NetworkGateway(ControllerAbstractInterface *controllerInterface);
	~NetworkGateway();

	/*!
	 *  Implements Gateway::id
	 */
	virtual std::string id();

	/*!
	 *  Implements Gateway::setConfig
	 */
	virtual bool setConfig(const std::string &config);

	/*!
	 *  Implements Gateway::activate
	 */
	virtual bool activate();

	/*! Implements Gateway::environment
	 */
	virtual std::string environment();

private:

        /*! Set container IP address
	*
	* Retrieves an IP from DHCP and sets the IP address.
	*
	* Note that a file on the system acts as a placeholder for the DHCP server.
	* The file keeps track of the highest used IP address.
	*
	* \return true  Upon success
	* \return false Upon failure
	*/
	bool setContainerIP();

        /*! Set route to default gateway
	*
	* Sets the route to the default gateway. The gateway IP address was parsed
	* during the setConfig() step.
	* To be able to access anything outside the container, this method must be
	* called after the network interface has been enabled. This is also true for
	* cases when a network interface that was previously enabled has been disabled
	* and then enabled again.
	*
	* \return true  Upon success
	* \return false Upon failure
	*/
	bool setDefaultGateway();

        /*! Enable the default network interface.
	*
	* Enables the network interface and calls setDefaultGateway().
	*
	* When this is done for the first time, i.e. during the first call to activate()
	* the IP and netmask are also set. During subsequent calls, this merely brings
	* up the existing network interface and calls setDefaultGateway().
	*
	* \return true  Upon success
	* \return false Upon failure
	*/
	bool up();

	bool down();

	/*! Generate and execute IPTables rules
	*
	* Parse the IPTables rules from configuration and execute these rules
	*
	* \param ipAddress	The IP address of the system
	* \param rules		String containing the IPTable rules
	*/
	bool setupIptables(const std::string &ipAddress, const std::string &rules);

	/*! Remove IPTables rules set up for a specific network iface
	*
	* This implementation is shady. What we do here is to look at the output of
	* iptables -L, look for our own IP, and then remove all rules matching our IP
	* from the FORWARD chain. There are several problems with this:
	*	- We don't lock the iptable to ensure the list we're comparing against
	*	  matches the actual list were removing from
	*	- We don't know whether we actually added the rules ourselves, or if
	*	  someone else did.
	* in short.. this function should be re-implemented in some other way where we
	* have atomic transactions for lookup and remove, and where we're also certain
	* we are actually the originators of the rule in question.
	*/
	bool teardownIptables(const std::string &ipAddress);

	/*! Place a bandwidth limitation on a network interface
	*
	* Limit the incoming and outgoing bandwidth on a network interface. The limits
	* are specified using params.tc_rate. The interface is specified using
	* params.net_iface_name.
	*
	* This function forks an observing process in order to detect the availability
	* of the network interface.
	*
	* \param ifaceName Network interface name
	* \param tcRate    TC rate
	* \return 0        Upon success
	* \return -EINVAL  Upon failure
	*/
	int limitIface(const std::string &ifaceName, const std::string &tcRate);

	/*! Remove the limits set by limit_iface()
	*
	* This will remove any limits set by limit_iface(). This call assumes the
	* network interface is available, and will not block until it can be obtained.
	*
	* \param  iface   name of interface
	* \return 0       upon success
	* \return -EINVAL upon failure to remove limit, or failure to allocate memory
	*/
	int clearIfaceLimits(char *iface);

	static int waitForDevice(const std::string &iface);

	std::string m_ip;
	std::string m_gateway;
	bool m_internetAccess;
	bool m_activatedOnce;

	bool printIfconfig();
	bool ping(const std::string &ip);
	std::string parseConfig(const std::string &config, const std::string &key);
	bool checkBridgeAvailability();

	bool selfTest();
};

#endif /* NETWORKGATEWAY_H */
