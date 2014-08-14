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

// Using sizeof inside the function would give size of pointer, not of array
void writeFile(char filename[], char contents[], size_t size) {
    int fd = mkstemp(filename);
    write(fd, contents, size - 1); // We don't need to write \0
    close(fd);
}

TEST(ConfigParserTest, TestInit) {
    // Nonexisting file
    char filename1[] = "tmpconfig1_XXXXXX";
    int fd = mkstemp(filename1);
    close(fd);
    unlink(filename1);

    ConfigParser config1;
    ASSERT_NE(0, config1.read(filename1));

    // Existing minimal JSON
    char filename2[] = "tmpconfig2_XXXXXX";
    char empty[] = "{ }";
    writeFile(filename2, empty, sizeof(empty));

    ConfigParser config2;
    ASSERT_EQ(0, config2.read(filename2));

    // Existing erroneous JSON
    char filename3[] = "tmpconfig3_XXXXXX";
    char erroneous[] = "{ \"foo\" : }";
    writeFile(filename3, erroneous, sizeof(erroneous));

    ConfigParser config3;
    ASSERT_NE(0, config1.read(filename3));
}

TEST(ConfigParserTest, TestReadSimple) {
    char *value = NULL;
    char filename[] = "tmpconfig_XXXXXX";
    char json[] = "{ \"test\": \"testvalue\" }";
    writeFile(filename, json, sizeof(json));

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
    char filename[] = "tmpconfig_XXXXXX";
    char json[] = "{ \"test\": [\"row1\", \"row2\"] }";
    writeFile(filename, json, sizeof(json));

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
