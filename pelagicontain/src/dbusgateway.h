/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DBUSGATEWAY_H
#define DBUSGATEWAY_H

#include <string>
#include <unistd.h>
#include "gateway.h"
#include "controllerinterface.h"

/*! DBus Gateway takes care of spawning and killing the DBus proxies
 */
class DBusGateway : public Gateway
{
public:
	enum ProxyType {SessionProxy, SystemProxy};

	/*! Spawn the proxy and use the supplied path for the socket
	*
	* \param  type       SessionProxy or SystemProxy
	*/
	DBusGateway(const ControllerAbstractInterface *controllerInterface,
		ProxyType type, const std::string &containerRoot,
		const std::string &name, const std::string &containerConfig);
	~DBusGateway();

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
	const char *typeString();
	const char *socketName();

	pid_t m_pid;
	std::string m_socket;
	ProxyType m_type;
};

#endif /* DBUSGATEWAY_H */
