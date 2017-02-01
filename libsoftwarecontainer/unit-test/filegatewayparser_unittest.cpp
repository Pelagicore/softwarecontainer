/*
 * Copyright (C) 2016-2017 Pelagicore AB
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
#include "gateway_parser_common.h"

using namespace softwarecontainer;

class FileGatewayParserTest : public GatewayParserCommon<std::string>
{
public:
    FileGatewayParser parser;
    FileGatewayParser::FileSetting result;
    std::vector<FileGatewayParser::FileSetting> settings;

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
    json_t *configJSON = convertToJSON(config);

    ASSERT_TRUE(parser.parseConfigElement(configJSON, result));
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
    json_t *configJSON = convertToJSON(config);

    ASSERT_FALSE(parser.parseConfigElement(configJSON, result));
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
    json_t *configJSON = convertToJSON(config);

    ASSERT_FALSE(parser.parseConfigElement(configJSON, result));
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
    json_t *configJSON = convertToJSON(config);

    ASSERT_FALSE(parser.parseConfigElement(configJSON, result));
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
    json_t *configJSON = convertToJSON(config);

    ASSERT_FALSE(parser.parseConfigElement(configJSON, result));
}

/*
 * Make sure configuration is rejected if the path in container is missing.
 */
TEST_F(FileGatewayParserTest, NoPathInContainer) {
    const std::string config =
        "{"
            "\"path-host\" : \"" + FILE_PATH + "\""
        "}";
    json_t *configJSON = convertToJSON(config);

    ASSERT_FALSE(parser.parseConfigElement(configJSON, result));
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
    json_t *configJSON = convertToJSON(config);

    ASSERT_FALSE(parser.parseConfigElement(configJSON, result));
}
