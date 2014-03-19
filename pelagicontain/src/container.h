/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONTAINER_H
#define CONTAINER_H

#include <string>
#include <vector>

#include "gateway.h"

/*! Container is an abstraction of the specific containment technology used.
 *
 * The Container class is meant to be an abstraction of the containment
 * technology, so that Pelagicontain can view it as a generic concept that
 * implements the specifics behind the conceptual phases of 'Pereload', 'Launch',
 * and 'Shutdown'.
 */
class Container
{
public:

	/*!
	 * \param configFile A path to the configuration file (including the file name)
	 */
	Container(const std::string &name, const std::string &configFile, const std::string &containerRoot);

	~Container();

	const char *name();

	std::vector<std::string> commands(const std::string &containedCommand,
		const std::vector<Gateway *> &gateways);

	/*
	 * Setup the container for a specific app
	 *
	 * Setup the container so that the app specific directories available inside the
	 * container.
	 */
	void setApplication(const std::string &appId);

	/*
	 * The unique name of the LXC container
	 */
	std::string m_name;

	/*
	 * The LXC configuration file for this container
	 */
	std::string m_configFile;
private:
	/*
	 * Check if path is a directory
	 */
	bool isDirectory(const std::string &path);

	/*
	 * Create a directory, and if successful append it to a list of dirs
	 * to be deleted in the dtor. Since nestled dirs will need to be
	 * deleted in reverse order to creation insert to the beginning of
	 * the list.
	 */
	bool createDirectory(const std::string &path);

	/*
	 * Create a bind mount. On success the mount will be added to a list of
	 * mounts that will be unmounted in the dtor.
	 */
	bool bindMountDir(const std::string &src, const std::string &dst);

	/*
	 * List of all, by the container, mounted directories. These directories
	 * should be unmounted in the destructor.
	 */
	std::vector<std::string> m_mounts;

	/*
	 * List of all, by the container, created directories. These directories
	 * should be deleted in the destructor.
	 */
	std::vector<std::string> m_dirs;

	std::string m_containerRoot;
	std::string m_mountDir;
};

#endif //CONTAINER_H
