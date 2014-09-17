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

class DLTGateway: public Gateway {

	LOG_DECLARE_CLASS_CONTEXT("DLTG", "DLT gateway");

public:

	static constexpr const char* ID = "dlt";

	DLTGateway(ControllerAbstractInterface &controllerInterface, SystemcallAbstractInterface &systemcallInterface,
			const std::string &gatewayDir, const std::string &name) :
			Gateway(controllerInterface), m_systemcallInterface(systemcallInterface) {
	}

	~DLTGateway() {
	}

	std::string id() override {
		return ID;
	}

	static constexpr const char* DLT_SOCKET_FOLDER = "/tmp/";
	static constexpr const char* SOCKET_FILE_NAME = "dlt";

	bool setConfig(const std::string &config) override {
		m_enabled = (config.length() != 0);
		log_info() << "Received config : " << config;
		return true;
	}

	bool activate() override {
		if (m_enabled) {
			log_error() << "enabling DLT gateway";
			std::string d = logging::StringBuilder() << DLT_SOCKET_FOLDER << "/" << SOCKET_FILE_NAME;
			std::string path = getContainer().bindMountFileInContainer(d, SOCKET_FILE_NAME);
			createSymLinkInContainer(path, d);
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

