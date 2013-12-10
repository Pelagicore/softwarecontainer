#include <iostream>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include "generators.h"

static void test_gen_ip()
{
	const char ip_addr_net[] = "192.168.0.";
	system ("rm -f /tmp/pelc_ifc");
	std::string ip = gen_ip_addr(ip_addr_net);
	g_assert (!ip.empty());
	g_assert (!g_strcmp0 (ip.c_str(), "192.168.0.2"));

	ip = gen_ip_addr(ip_addr_net);
	g_assert (!g_strcmp0 (ip.c_str(), "192.168.0.3"));

	system("echo 253 > /tmp/pelc_ifc");
	ip = gen_ip_addr(ip_addr_net);
	g_assert (!g_strcmp0 (ip.c_str(), "192.168.0.254"));

	ip = gen_ip_addr(ip_addr_net);
	g_assert (!g_strcmp0 (ip.c_str(), "192.168.0.2"));
}

static void test_ct_name()
{
	std::string name1 = gen_ct_name();
	std::string name2 = gen_ct_name();
	/* This can fail with some certainty */
	g_assert (!g_strcmp0 (name1.c_str(), name2.c_str()));

	/* Wait for new random seed */
	usleep(1000);
	std::string name3 = gen_ct_name();
	g_assert (g_strcmp0 (name1.c_str(), name3.c_str()));
	
}

int main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/config/gen_ip",  test_gen_ip);
	g_test_add_func ("/config/ct_name", test_ct_name);
	return g_test_run ();
}
