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

#include "config.h"

Config::Config() : root(0)
{
}

Config::~Config()
{
	if (root) {
		json_decref(root);
	}
}

int Config::read(const char *path)
{
	if (root) {
		log_error("Already loaded configuration!");
		return -EINVAL;
	}

	json_error_t error;

	root = json_load_file(path, 0, &error);

	if (!root) {
		log_error("error: on line %d: %s", error.line, error.text);
		return -EINVAL;
	}

	debug("Using config file %s", path);
	return 0;
}

char *Config::getString(const char *property)
{
	json_t *element = NULL;

	if (root == NULL) {
		debug ("Root JSON object is not initialized");
		return NULL;
	}

	element = json_object_get(root, property);
	if (json_is_string (element)) {
		debug ("%s is called on a string");
		return strdup (json_string_value (element));
	} else if (json_is_array (element)) {
		debug ("%s is called on an array");
		size_t  len    = json_array_size (element);
		int     buflen = 100;
		int     j      = 0;
		char   *buf    = (char*)calloc (sizeof (char), buflen);

		for (size_t i = 0; i < len; i++) {
			json_t *line    = json_array_get (element, i);
			const char *strline = json_string_value (line);
			int     linelen = 0;

			/* Entire array must be strings */
			if (!json_is_string (line)) {
				debug ("line %d is not a string!", i);
				free (buf);
				return NULL;
			}

			/* Ensure new string fits in buffer */
			linelen = strlen (strline);
			if (j + linelen > buflen) {
				debug ("buf is %d, and line is %d", buflen, linelen);
				buflen = (j + strlen (strline)) * 2;
				char *newbuf = (char*)calloc (sizeof (char), buflen);

				strncpy (newbuf, buf, buflen);
				free (buf);
				buf = newbuf;
			}
			j += linelen;

			/* We already ensured this fits */
			if (i > 0)
				strcat (buf, "\n");

			strcat (buf, strline);
		}

		return buf;
	}

	debug ("Called on an unknown type");
	return NULL;
}
