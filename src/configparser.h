/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include "string.h"
#include "jansson.h"
#include "errno.h"

/*! \brief  Key-value configuration system.
 *  \file   config.h
 *
 * The system is used to retrieve values based
 * on key identifiers supplied to ConfigParser::get* functions.
 */

class ConfigParser {
public:
	ConfigParser();
	~ConfigParser();

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

#endif /* CONFIGPARSER_H */
