/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include <stdlib.h>
#include <string.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "configparser.h"

TEST(ConfigParserTest, TestInit) {
    // Nonexisting file
    system ("rm -f /tmp/config");
    ConfigParser config1;
    ASSERT_NE(0, config1.read("/tmp/config"));

    // Existing minimal JSON
    ConfigParser config2;
    system("echo '{ }' > /tmp/config");
    ASSERT_EQ(0, config2.read("/tmp/config"));

    // Existing erroneous JSON
    ConfigParser config3;
    system("echo '{ \"foo\" : }' > /tmp/config");
    ASSERT_NE(0, config1.read("/tmp/config"));
}

TEST(ConfigParserTest, TestReadSimple) {
    char *value = NULL;
	system ("echo '{ \"test\": \"testvalue\" }' > /tmp/config");
    ConfigParser config;
    ASSERT_EQ(0, config.read("/tmp/config"));

    value = config.getString("test");
    ASSERT_FALSE(value == NULL);
    std::string val(value);
    ASSERT_EQ(val, "testvalue");

    value = config.getString("test2");
    ASSERT_TRUE(value == NULL);
}

TEST(ConfigParserTest, TestReadMultiline) {
	system ("echo '{ \"test\": [\"row1\", \"row2\"] }' > /tmp/config");
    ConfigParser config;
    ASSERT_EQ(0, config.read("/tmp/config"));

    // We can get the value
    char *value = config.getString("test");
    ASSERT_FALSE(value == NULL);

    // And it should be a string delimited by \n containing two values
    char * token;
    token = strtok(value, "\n");
    ASSERT_FALSE(token == NULL);
    std::string val1(token);
    ASSERT_EQ(val1, "row1");

    token = strtok(NULL, "\n");
    ASSERT_FALSE(token == NULL);
    std::string val2(token);
    ASSERT_EQ(val2, "row2");

    // Now there should be no more
    token = strtok(NULL, "\n");
    ASSERT_TRUE(token == NULL);
}
