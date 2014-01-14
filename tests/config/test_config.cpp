#include <glib.h>
#include <stdlib.h>
#include "config.h"

static void test_initialize ()
{
	system ("rm -f /tmp/config");
	Config config1;
	g_assert (config1.read("/tmp/config") != 0);

	Config config2;
	system ("echo '{ }' > /tmp/config");
	g_assert (config2.read("/tmp/config") == 0);
}

static void test_read_simple_string ()
{
	gchar *value = NULL;
	system ("echo '{ \"test\": \"testvalue\" }' > /tmp/config");
	Config config;
	g_assert (config.read("/tmp/config") == 0);

	value = config.getString("test");
	g_assert (value != NULL);
	g_assert (g_strcmp0 (value, "testvalue") == 0);
}

static void test_read_multiline_string ()
{
	gchar *value = NULL;
	system ("echo '{ \"test\": [\"row1\", \"row2\"] }' > /tmp/config");
	Config config;
	config.read("/tmp/config");

	value = config.getString("test");
	g_assert (value != NULL);
}

int main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/config/initialize",       test_initialize);
	g_test_add_func ("/config/simple_string",    test_read_simple_string);
	g_test_add_func ("/config/multiline_string", test_read_multiline_string);
	return g_test_run ();
}
