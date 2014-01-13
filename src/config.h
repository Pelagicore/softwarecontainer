/*
 * Copyright (C) 2013, Pelagicore AB <jonatan.palsson@pelagicore.com>
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

#ifndef CONFIG_H
#define CONFIG_H

#include "string.h"
#include "jansson.h"
#include "errno.h"
#include "debug.h"

/*! \brief  Key-value configuration system.
 *  \file   config.h
 *
 * The system is used to retrieve values based
 * on key identifiers supplied to Config::get* functions.
 */

class Config {
public:
	Config();
	~Config();

	/*! \brief Read and parse the configuration file
	*
	* There can only be one of this system, which means this function should only
	* be run once. Running this function more than once is erroneous.
	*
	* \param  path The full, absolute path to the configuration file (including
	*         filename)
	* \return 0 upon success
	* \return -EINVAL upon attempted re-initialization
	* \return -EINVAL upon malformed config
	*/
	int read(const char *path);

	/*! \brief Retrieve a string from the config
	*
	* Retrieve a single or multi-line string from the configuration.
	*
	* \param  property The identifier for the string property to retrieve
	* \return NULL     When config_initialize(char *) has not been called
	* \return NULL     When property does not identify a config value
	* \return          A string upon successful retrieval of config value
	*/
	char *getString(const char *property);

private:
	json_t *root;
};

#endif //CONFIG_H
