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

    json_error_t err;
    json_t *configJSON;

    void convertToJSON(const std::string config)
    {
        configJSON = json_loads(config.c_str(), 0, &err);
        ASSERT_TRUE(configJSON != NULL);
    }

    const std::string FILE_CONTENT = "ahdkhqweuyreqiwenomndlaskmd";
    const std::string FILE_PATH = "/tmp/filename.txt";
    const std::string CONTAINER_PATH = "/filename.txt";
    const std::string ENV_VAR_NAME = "TEST_ENVIRONMENT_VARIABLE_NAME";
    const std::string PREFIX = "TEST_PREFIX";
    const std::string SUFFIX = "TEST_SUFFIX";
};

/*
 * Test that a minimal working conf is accepted
 */
TEST_F(FileGatewayParserTest, TestMinimalWorkingConf) {
    const std::string config =
        "{"
            "  \"path-host\" : \"" + FILE_PATH + "\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseFileGatewayConfigElement(configJSON, result));
}

/*
 * Test that things that should be strings are not accepted if they are not strings
 */
TEST_F(FileGatewayParserTest, TestBadStrings) {
    const std::string config =
        "{"
            "  \"path-host\": true"
            ", \"path-container\": 243"
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseFileGatewayConfigElement(configJSON, result));
    ASSERT_TRUE(result.pathInHost.empty());
    ASSERT_TRUE(result.pathInContainer.empty());
}

/*
 * Test that things that should be bools are not accepted if they are not bools
 */
TEST_F(FileGatewayParserTest, TestBadBools) {
    const std::string config =
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
            ", \"read-only\": 0"
            ", \"create-symlink\": []"
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseFileGatewayConfigElement(configJSON, result));
    ASSERT_FALSE(result.readOnly);
    ASSERT_FALSE(result.createSymlinkInContainer);
}

/*
 * Make sure configuration is rejected if no path in host is provided
 */
TEST_F(FileGatewayParserTest, TestNoPathInHost) {
    const std::string config = 
        "{"
            "\"path-container\" : \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseFileGatewayConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if the path in host is an empty string
 */
TEST_F(FileGatewayParserTest, TestEmptyPathInHost) {
    const std::string config = 
        "{"
            "  \"path-host\": \"\""
            ", \"path-container\" : \"" + CONTAINER_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseFileGatewayConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if the path in container is missing.
 */
TEST_F(FileGatewayParserTest, TestNoPathInContainer) {
    const std::string config =
        "{"
            "\"path-host\" : \"" + FILE_PATH + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseFileGatewayConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if path in container is an empty string
 */
TEST_F(FileGatewayParserTest, TestEmptyPathInContainer) {
    const std::string config = 
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseFileGatewayConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if env variable prefix is given without an env var name
 */
TEST_F(FileGatewayParserTest, TestEnvPrefixWithoutEnvName) {
    const std::string config = 
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
            ", \"env-var-prefix\": \"" + PREFIX + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseFileGatewayConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if env variable suffix is given without an env var name
 */
TEST_F(FileGatewayParserTest, TestEnvSuffixWithoutEnvName) {
    const std::string config = 
        "{"
            "  \"path-host\": \"" + FILE_PATH + "\""
            ", \"path-container\": \"" + CONTAINER_PATH + "\""
            ", \"env-var-suffix\": \"" + SUFFIX + "\""
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseFileGatewayConfigElement(configJSON, result));
}
