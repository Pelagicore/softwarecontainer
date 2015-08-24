/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "generators.h"

TEST(GeneratorTest, TestIPCounter) {
    Generator gen;
    const char ip_addr_net[] = "192.168.0.";

    // We start from 0 (no really, we start from 2)
    std::string ip = gen.gen_ip_addr(ip_addr_net);
    ASSERT_FALSE(ip.empty());
    ASSERT_EQ(ip, "192.168.0.2");

    ip = gen.gen_ip_addr(ip_addr_net);
    ASSERT_EQ(ip, "192.168.0.3");

    // Update so the counter is 253
    for (int i = 3; i < 253; i++) {
        gen.gen_ip_addr(ip_addr_net);
    }

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
}
