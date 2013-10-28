#include "string.h"
#include "jansson.h"
#include "errno.h"

json_t *root = NULL;

int initialize (char *path) {
	json_error_t error;

	root = json_load_file(path, 0, &error);

	if(!root) {
		g_printerr("error: on line %d: %s\n", error.line, error.text);
		return 1;
	}

	return 0;
}

char *config_get_string (char *property) {
	json_t *element = NULL;

	element = json_object_get(root, property);
	if (!json_is_string (element)) {
		return NULL;
	}
}
