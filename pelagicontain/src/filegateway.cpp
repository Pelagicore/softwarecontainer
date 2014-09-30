/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include <string>
#include "filegateway.h"

bool FileGateway::setConfig(const std::string &config) {

	log_info() << "called";
	JSonParser parser(config);

	std::vector<JSonParser> elements;
	parser.read("files", elements);

	for(auto& parser:elements) {

		log_info() << "element";

		std::string pathInHost;
		parser.readString("path-host", pathInHost);

		std::string pathInContainer;
		parser.readString("path-container", pathInContainer);

		bool createSymlinkInContainer = false;
		parser.read("create-symlink", createSymlinkInContainer);

		bool readOnly = false;
		parser.read("read-only", readOnly);

//		std:string envVariableName = false;
//		parser.read("env-var-name", readOnly);

		std::string envVarName;
		parser.read("env-var-name", envVarName);

		std::string envVarValue;
		parser.read("env-var-value", envVarValue);

		std::string path = getContainer().bindMountFileInContainer(pathInHost,  pathInContainer, readOnly);

		if (envVarName.size() != 0) {
			char value[1024];
			snprintf(value, sizeof(value), envVarValue.c_str(), path.c_str());
			setEnvironmentVariable(envVarName, value);
		}

		if (createSymlinkInContainer)
			createSymLinkInContainer(path, pathInHost);

	}

	return true;
}
