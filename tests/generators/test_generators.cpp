#include <glib.h>
#include <stdlib.h>
#include "generators.h"

static void test_gen_ip()
{
	const char ip_addr_net[] = "192.168.0.";
	system ("rm -f /tmp/pelc_ifc");
	char *ip = gen_ip_addr(ip_addr_net);
	g_assert (ip);

	g_assert (!g_strcmp0 (ip, "192.168.0.2"));
	free(ip);

	ip = gen_ip_addr(ip_addr_net);
	g_assert (!g_strcmp0 (ip, "192.168.0.3"));
	free(ip);

	system("echo 253 > /tmp/pelc_ifc");
	ip = gen_ip_addr(ip_addr_net);
	g_assert (!g_strcmp0 (ip, "192.168.0.254"));
	free(ip);

	ip = gen_ip_addr(ip_addr_net);
	g_assert (!g_strcmp0 (ip, "192.168.0.2"));
	free(ip);
}

int main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/config/gen_ip",       test_gen_ip);
	return g_test_run ();
}
