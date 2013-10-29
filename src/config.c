#include "string.h"
#include "jansson.h"
#include "errno.h"

#undef DEBUGCONFIG

#ifdef DEBUGCONFIG
	#define debug(...) printf("DEBUG: " __VA_ARGS__)
#endif
#ifndef DEBUGCONFIG
	#define debug(...)
#endif


       int     DEBUG_config = 1;
static json_t *root         = NULL;

int config_initialize (char *path) {
	json_error_t error;

	root = json_load_file(path, 0, &error);

	if(!root) {
		printf("error: on line %d: %s\n", error.line, error.text);
		return -EINVAL;
	}

	return 0;
}

void config_destroy () {
	json_decref (root);
}

char *config_get_string (char *property) {
	json_t *element = NULL;

	if (root == NULL) {
		debug ("Root JSON object is not initialized");
		return NULL;
	}

	element = json_object_get(root, property);
	if (json_is_string (element)) {
		debug ("%s is called on a string\n", __FUNCTION__);
		return strdup (json_string_value (element));
	} else if (json_is_array (element)) {
		debug ("%s is called on an array\n", __FUNCTION__);
		size_t  len    = json_array_size (element);
		size_t  buflen = 100;
		int     i      = 0;
		int     j      = 0;
		char   *buf    = calloc (sizeof (char), buflen);

		for (i = 0; i < len; i++) {
			      json_t *line    = json_array_get (element, i);
			const char   *strline = json_string_value (line);
			      int     linelen = 0;

			/* Entire array must be strings */
			if (!json_is_string (line)) {
				debug ("line %d is not a string!\n", i);
				free (buf);
				return NULL;
			}

			/* Ensure new string fits in buffer */
			linelen = strlen (strline);
			if (j + linelen > buflen) {
				debug ("buf is %d, and line is %d\n", buflen, linelen);
				buflen = (j + strlen (strline)) * 2;
				char *newbuf = calloc (sizeof (char), buflen);

				strncpy (newbuf, buf, buflen);
				free (buf);
				buf = newbuf;
				j += linelen;
			}

			/* We already ensured this fits */
			if (i > 0)
				strcat (buf, "\n");

			strcat (buf, strline);
		}

		return buf;
	}

	debug ("%s is called on an unknown type\n", __FUNCTION__);
	return NULL;
}
