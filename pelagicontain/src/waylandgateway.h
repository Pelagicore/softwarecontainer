/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#pragma once

#include <string>
#include <unistd.h>
#include "gateway.h"
#include "controllerinterface.h"
#include "systemcallinterface.h"

class WaylandGateway: public Gateway {

	LOG_DECLARE_CLASS_CONTEXT("Wayl", "Wayland gateway");

public:

	static constexpr const char* ID = "wayland";

	WaylandGateway(ControllerAbstractInterface &controllerInterface, SystemcallAbstractInterface &systemcallInterface,
			const std::string &gatewayDir, const std::string &name) :
			Gateway(controllerInterface), m_systemcallInterface(systemcallInterface) {
	}

	~WaylandGateway() {
	}

	std::string id() override {
		return ID;
	}

	static constexpr const char* WAYLAND_RUNTIME_DIR_VARIABLE_NAME= "XDG_RUNTIME_DIR";
	static constexpr const char* SOCKET_FILE_NAME = "wayland-0";

	bool setConfig(const std::string &config) override {
		m_enabled = (config.length() != 0);
		log_info() << "Received config : " << config;
		return true;
	}

	bool activate() override {
		if (m_enabled) {
			log_info() << "enabling Wayland gateway";
			const char* dir = getenv(WAYLAND_RUNTIME_DIR_VARIABLE_NAME);
			if (dir != nullptr) {
				std::string d = logging::StringBuilder() << dir << "/" << SOCKET_FILE_NAME;
				std::string path = getContainer().bindMountFileInContainer(d,  SOCKET_FILE_NAME);
				getController().setEnvironmentVariable(WAYLAND_RUNTIME_DIR_VARIABLE_NAME, parentPath(path));
			} else
				return false;
		}

		return true;
	}

	bool teardown() override {
		return true;
	}

private:

	SystemcallAbstractInterface &m_systemcallInterface;
	bool m_enabled = false;
};

