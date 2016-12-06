/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

#include "gateway/files/filegatewayparser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class FileGatewayParserTest : public ::testing::Test
{
public:
    FileGatewayParser parser;
    FileGatewayParser::FileSetting result;
    std::vector<FileGatewayParser::FileSetting> settings;

    json_error_t err;
    json_t *configJSON;

    void convertToJSON(const std::string config)
    {
        configJSON = json_loads(config.c_str(), 0, &err);
        ASSERT_TRUE(configJSON != NULL);
    }

    const std::string FILE_PATH = "/tmp/filename.txt";
    const std::string CONTAINER_PATH = "/filename.txt";
};

/*
 * Test that a minimal working conf is accepted
 */
TEST_F(FileGatewayParserTest, MinimalWorkingConf) {
    const std::string config =
        "{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
}

/*
 * Test that things that should be strings are not accepted if they are not strings
 */
TEST_F(FileGatewayParserTest, BadStrings) {
    const std::string config =
        "{"
            "  \"path-host\": true"
            ", \"path-container\": 243"
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseConfigElement(configJSON, result));
    ASSERT_TRUE(result.pathInHost.empty());
    ASSERT_TRUE(result.pathInContainer.empty());
}

/*
 * Test that things that should be bools are not accepted if they are not bools
 */
TEST_F(FileGatewayParserTest, BadBools) {
    const std::string config =
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
            ", \"read-only\": 0"
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseConfigElement(configJSON, result));
    ASSERT_FALSE(result.readOnly);
}

/*
 * Make sure configuration is rejected if no path in host is provided
 */
TEST_F(FileGatewayParserTest, NoPathInHost) {
    const std::string config = 
        "{"
            "\"path-container\" : \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if the path in host is an empty string
 */
TEST_F(FileGatewayParserTest, EmptyPathInHost) {
    const std::string config = 
        "{"
            "  \"path-host\": \"\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if the path in container is missing.
 */
TEST_F(FileGatewayParserTest, NoPathInContainer) {
    const std::string config =
        "{"
            "\"path-host\" : \"" + FILE_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if path in container is an empty string
 */
TEST_F(FileGatewayParserTest, EmptyPathInContainer) {
    const std::string config = 
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseConfigElement(configJSON, result));
}

/*
 * Using the same configuration twice is ok, as long as the container path
 * is the same
 */
TEST_F(FileGatewayParserTest, SameConfigTwice) {
    const std::string config = 
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
    ASSERT_EQ(ReturnCode::SUCCESS, parser.matchEntry(result, settings));
    ASSERT_EQ(1u, settings.size());

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
    ASSERT_EQ(ReturnCode::SUCCESS, parser.matchEntry(result, settings));
    ASSERT_EQ(1u, settings.size());
}

/*
 * Using the same paths but with different values on read-only is fine, since we
 * will use the most permissive setting.
 */
TEST_F(FileGatewayParserTest, ReadOnlyPrecedence) {
    const std::string config1 =
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
            ", \"read-only\": true"
        "}";
    convertToJSON(config1);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
    ASSERT_EQ(ReturnCode::SUCCESS, parser.matchEntry(result, settings));
    ASSERT_EQ(1u, settings.size());

    const std::string config2 =
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
            ", \"read-only\": false"
        "}";
    convertToJSON(config2);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
    ASSERT_EQ(ReturnCode::SUCCESS, parser.matchEntry(result, settings));
    ASSERT_EQ(1u, settings.size());
    ASSERT_EQ(settings.at(0).readOnly, false);
}

/*
 * Testing same host path but different container paths is fine, we will
 * mount the same file to two places, no problems.
 */
TEST_F(FileGatewayParserTest, SameHostPathDifferentContainerPath) {
    const std::string config1 =
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config1);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
    ASSERT_EQ(ReturnCode::SUCCESS, parser.matchEntry(result, settings));
    ASSERT_EQ(1u, settings.size());

    const std::string config2 =
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "2\""
        "}";
    convertToJSON(config2);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
    ASSERT_EQ(ReturnCode::SUCCESS, parser.matchEntry(result, settings));
    ASSERT_EQ(2u, settings.size());
}

TEST_F(FileGatewayParserTest, DifferentHostPathSameContainerPath) {
    const std::string config1 =
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config1);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
    ASSERT_EQ(ReturnCode::SUCCESS, parser.matchEntry(result, settings));
    ASSERT_EQ(1u, settings.size());

    const std::string config2 =
        "{"
            "  \"path-host\": \"" + FILE_PATH + "2\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config2);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseConfigElement(configJSON, result));
    ASSERT_EQ(ReturnCode::FAILURE, parser.matchEntry(result, settings));
    ASSERT_EQ(1u, settings.size());
}
