/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <stdlib.h>
#include <unistd.h>

#include "generators.h"

TEST(GeneratorTest, TestIPCounter) {
    Generator gen;
	const char ip_addr_net[] = "192.168.0.";
	system ("rm -f /tmp/pelc_ifc");
    
    std::string ip = gen.gen_ip_addr(ip_addr_net);
    ASSERT_FALSE(ip.empty());
    ASSERT_EQ(ip, "192.168.0.2");

    ip = gen.gen_ip_addr(ip_addr_net);
    ASSERT_EQ(ip, "192.168.0.3");

	system("echo 253 > /tmp/pelc_ifc");
    ip = gen.gen_ip_addr(ip_addr_net);
    ASSERT_EQ(ip, "192.168.0.254");

    ip = gen.gen_ip_addr(ip_addr_net);
    ASSERT_EQ(ip, "192.168.0.2");
}

TEST(GeneratorTest, TestNameGen) {
    Generator gen;
    
    // No time = No new seed
    std::string name1 = gen.gen_ct_name();
    std::string name2 = gen.gen_ct_name();
    ASSERT_EQ(name1, name2);

    usleep(1000);
    std::string name3 = gen.gen_ct_name();
    ASSERT_NE(name1, name3);
    ASSERT_NE(name2, name3);
}
