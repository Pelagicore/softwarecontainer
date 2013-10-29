#include "glib.h"
#include "config.h"
#include "stdlib.h"



static void test_initialize ()
{
	system ("rm -f /tmp/config");
	g_assert (config_initialize ("/tmp/config") != 0);
	config_destroy ();

	system ("echo '{ }' > /tmp/config");
	g_assert (config_initialize ("/tmp/config") == 0);
	config_destroy ();
}

static void test_read_simple_string ()
{
	gchar *value = NULL;
	system ("echo '{ \"test\": \"testvalue\" }' > /tmp/config");
	config_initialize ("/tmp/config");

	value = config_get_string ("test");
	g_assert (value != NULL);
	g_assert (g_strcmp0 (value, "testvalue") == 0);

	config_destroy ();
}

static void test_read_multiline_string ()
{
	gchar *value = NULL;
	system ("echo '{ \"test\": [\"row1\", \"row2\"] }' > /tmp/config");
	config_initialize ("/tmp/config");

	value = config_get_string ("test");
	g_assert (value != NULL);

	config_destroy ();

}

int main (int argc, int **argv)
{
	g_test_init (&argc, (char ***)&argv, NULL);
	g_test_add_func ("/config/initialize",       test_initialize);
	g_test_add_func ("/config/simple_string",    test_read_simple_string);
	g_test_add_func ("/config/multiline_string", test_read_multiline_string);
	return g_test_run ();
}
