/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "debug.h"
#include "config.h"

Config::Config() :
	root(0)
{
}

Config::~Config()
{
	if (root)
		json_decref(root);
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

	log_debug("Using config file %s", path);
	return 0;
}

char *Config::getString(const char *property)
{
	json_t *element = NULL;

	if (root == NULL) {
		log_debug("Root JSON object is not initialized");
		return NULL;
	}

	element = json_object_get(root, property);
	if (json_is_string(element)) {
		return strdup(json_string_value(element));
	} else if (json_is_array(element)) {
		size_t len = json_array_size(element);
		int buflen = 100;
		int j = 0;
		char *buf = (char*)calloc(sizeof(char), buflen);

		for (size_t i = 0; i < len; i++) {
			json_t *line = json_array_get(element, i);
			const char *strline = json_string_value(line);
			int linelen = 0;

			/* Entire array must be strings */
			if (!json_is_string(line)) {
				log_debug("line %d is not a string!", i);
				free(buf);
				return NULL;
			}

			/* Ensure new string fits in buffer */
			linelen = strlen(strline);
			if (j + linelen > buflen) {
				buflen = (j + strlen(strline)) * 2;
				char *newbuf = (char*)calloc(sizeof(char), buflen);

				strncpy(newbuf, buf, buflen);
				free(buf);
				buf = newbuf;
			}
			j += linelen;

			/* We already ensured this fits */
			if (i > 0)
				strcat(buf, "\n");

			strcat(buf, strline);
		}

		return buf;
	}

	log_debug("Called on an unknown type");
	return NULL;
}
