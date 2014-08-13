/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "configparser.h"

TEST(ConfigParserTest, TestInit) {
    // Nonexisting file
    char filename1[18] = "tmpconfig1_XXXXXX";
    int fd = mkstemp(filename1);
    close(fd);
    unlink(filename1);

    ConfigParser config1;
    ASSERT_NE(0, config1.read(filename1));

    // Existing minimal JSON
    char filename2[18] = "tmpconfig2_XXXXXX";
    fd = mkstemp(filename2);
    char empty[4] = "{ }";
    write(fd, empty, 3);
    close(fd);

    ConfigParser config2;
    ASSERT_EQ(0, config2.read(filename2));

    // Existing erroneous JSON
    char filename3[18] = "tmpconfig3_XXXXXX";
    fd = mkstemp(filename3);
    char erroneous[12] = "{ \"foo\" : }";
    write(fd, erroneous, 11);
    close(fd);

    ConfigParser config3;
    ASSERT_NE(0, config1.read(filename3));
}

TEST(ConfigParserTest, TestReadSimple) {
    char *value = NULL;
    char filename[17] = "tmpconfig_XXXXXX";
    int fd = mkstemp(filename);
    char json[24] = "{ \"test\": \"testvalue\" }";
    write(fd, json, 23);
    close(fd);

    ConfigParser config;
    ASSERT_EQ(0, config.read(filename));

    value = config.getString("test");
    ASSERT_FALSE(value == NULL);
    std::string val(value);
    ASSERT_EQ(val, "testvalue");

    value = config.getString("test2");
    ASSERT_TRUE(value == NULL);
}

TEST(ConfigParserTest, TestReadMultiline) {
    char filename[17] = "tmpconfig_XXXXXX";
    int fd = mkstemp(filename);
    char json[29] = "{ \"test\": [\"row1\", \"row2\"] }";
    write(fd, json, 28);
    close(fd);

    ConfigParser config;
    ASSERT_EQ(0, config.read(filename));

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
