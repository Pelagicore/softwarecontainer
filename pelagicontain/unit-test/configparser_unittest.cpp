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

void writeStringToFile(char filename[], const char *contents) {
    int fd = mkstemp(filename);
    write(fd, contents, strnlen(contents, 50));
    close(fd);
}

TEST(ConfigParserTest, TestInit) {
    // Nonexisting file
    char filename1[] = "tmpconfig1_XXXXXX";
    int fd = mkstemp(filename1);
    close(fd);
    unlink(filename1);

    ConfigParser config1;
    EXPECT_NE(0, config1.read(filename1));

    // Existing minimal JSON
    char filename2[] = "tmpconfig2_XXXXXX";
    const char empty[] = "{ }";
    writeStringToFile(filename2, empty);

    ConfigParser config2;
    EXPECT_EQ(0, config2.read(filename2));

    // Existing erroneous JSON
    char filename3[] = "tmpconfig3_XXXXXX";
    const char erroneous[] = "{ \"foo\" : }";
    writeStringToFile(filename3, erroneous);

    ConfigParser config3;
    EXPECT_NE(0, config3.read(filename3));
}

TEST(ConfigParserTest, TestReadSimple) {
    char *value = NULL;
    char filename[] = "tmpconfig_XXXXXX";
    const char json[] = "{ \"test\": \"testvalue\" }";
    writeStringToFile(filename, json);

    ConfigParser config;
    ASSERT_EQ(0, config.read(filename));

    value = config.getString("test");
    EXPECT_FALSE(value == NULL);
    std::string val(value);
    EXPECT_EQ(val, "testvalue");

    value = config.getString("test2");
    EXPECT_TRUE(value == NULL);
}

TEST(ConfigParserTest, TestReadMultiline) {
    char filename[] = "tmpconfig_XXXXXX";
    const char json[] = "{ \"test\": [\"row1\", \"row2\"] }";
    writeStringToFile(filename, json);

    ConfigParser config;
    ASSERT_EQ(0, config.read(filename));

    // We can get the value
    char *value = config.getString("test");
    EXPECT_FALSE(value == NULL);

    // And it should be a string delimited by \n containing two values
    char * token;
    token = strtok(value, "\n");
    EXPECT_FALSE(token == NULL);
    std::string val1(token);
    EXPECT_EQ(val1, "row1");

    token = strtok(NULL, "\n");
    EXPECT_FALSE(token == NULL);
    std::string val2(token);
    EXPECT_EQ(val2, "row2");

    // Now there should be no more
    token = strtok(NULL, "\n");
    EXPECT_TRUE(token == NULL);
}
