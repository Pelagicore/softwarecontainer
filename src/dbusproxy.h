/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef DBUSPROXY_H
#define DBUSPROXY_H

#include <string>
#include <unistd.h>
#include "gateway.h"

/*! \brief DBus Proxy
 *  \file dbusproxy.h
 *
 *  Takes care of spawning and killing the DBus proxies
 */
class DBusProxy : public Gateway
{
public:
	enum ProxyType {SessionProxy, SystemProxy};

	/*! Spawn the proxy and use the supplied path for the socket
	*
	* \param  socket     path to the socket file to use. File is created.
	* \param  config     path to configuration file for proxy
	* \param  type       SessionProxy or SystemProxy
	*/
	DBusProxy(const char *socket, const char *config, ProxyType type);
	~DBusProxy();

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
	const char *m_socket;
	ProxyType m_type;
};

#endif /* DBUSPROXY_H */
