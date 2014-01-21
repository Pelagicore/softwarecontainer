/*
 * Copyright (C) 2013, Pelagicore AB <erik.boto@pelagicore.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

/*! \brief DBus Proxy
 *  \file dbusproxy.h
 *
 *  Takes care of spawning and killing the DBus proxies
 */

#ifndef DBUSPROXY_H
#define DBUSPROXY_H

#include <string>
#include <unistd.h>
#include "gateway.h"

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

#endif //DBUSPROXY_H
