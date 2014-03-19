/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include "dbusgateway.h"
#include "debug.h"

DBusGateway::DBusGateway(ControllerAbstractInterface *controllerInterface,
	ProxyType type, const std::string &gatewayDir,
	const std::string &name, const std::string &containerConfig):
	Gateway(controllerInterface),
	m_type(type)
{
	/* TODO: The config should not be set here, it should be set
	 * when Pelagicontain calls setConfig() on this object. This
	 * means dbus-proxy needs to be modified to not take a config
	 * file as argument which it bases it's config on, rather it
	 * should somehow get the config set as a result of the call
	 * to setConfig()
	 */
	if (m_type == SessionProxy) {
		m_socket = gatewayDir
			+ std::string("/sess_")
			+ name
			+ std::string(".sock");
	} else {
		m_socket = gatewayDir
			+ std::string("/sys_")
			+ name
			+ std::string(".sock");
	}

	log_debug("Spawning %s proxy, socket: %s, config: %s",
		typeString(), m_socket.c_str(), containerConfig.c_str());

	m_pid = fork();
	if (m_pid == 0) { /* child */
		/* execlp only returns on errors */
		execlp("dbus-proxy", "dbus-proxy", m_socket.c_str(), typeString(),
			containerConfig.c_str(), NULL);
		log_error("Unable to spawn %s proxy!", typeString());
		/* Kill our clone, otherwise we get multiple running processes */
		exit(1);
	} else if (m_pid == -1) {
		log_error("Failed to fork!");
	}
}

DBusGateway::~DBusGateway()
{
	if (kill(m_pid, SIGTERM) == -1) {
		log_error("Failed to kill %s proxy!", typeString());
	} else {
		log_debug("Killed %s proxy!", typeString());
	}

	if (remove(m_socket.c_str()) == -1) {
		log_error("Failed to remove %s proxy socket!", typeString());
	} else {
		log_debug("Removed %s proxy socket!", typeString());
	}
}

std::string DBusGateway::id()
{
	return "dbus-proxy";
}

bool DBusGateway::setConfig(const std::string &config)
{
	return true;
}

bool DBusGateway::activate()
{
	return true;
}

const char *DBusGateway::typeString()
{
	if (m_type == SessionProxy) {
		return "session";
	} else {
		return "system";
	}
}

const char *DBusGateway::socketName()
{
	// Return the filename after stripping directory info
	std::string socket(m_socket.c_str());
	return socket.substr(socket.rfind('/') + 1).c_str();
}

std::string DBusGateway::environment()
{
	log_debug("Requesting environment for %s with socket %s",
		typeString(), m_socket.c_str());

	std::string env;
	env += "DBUS_";
	if (m_type == SessionProxy) {
		env += "SESSION";
	} else {
		env += "SYSTEM";
	}
	env += "_BUS_ADDRESS=unix:path=";
	env += "/gateways/";
	env += socketName();

	return env;
}
