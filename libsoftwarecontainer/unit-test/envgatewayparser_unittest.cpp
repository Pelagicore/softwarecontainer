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


#include "gateway/environment/envgatewayparser.h"
#include "gateway_parser_common.h"

class EnvGatewayParserTest : public GatewayParserCommon<std::string>
{
public:
    EnvironmentGatewayParser parser;
    EnvironmentGatewayParser::EnvironmentVariable result;
    EnvironmentVariables store;

    // Sample values
    std::string name = "XDG_RUNTIME_DIR";
    std::string value = "/run/user/1000";
    std::string separator = ":";
};

/*
 * Test that not supplying a name fails
 */
TEST_F(EnvGatewayParserTest, NoName) {
    const std::string config = "{ \"value\": \"" + value + "\" }";
    json_t *configJSON = convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_TRUE(result.first.empty());
}

/*
 * Test that not supplying a value fails
 */
TEST_F(EnvGatewayParserTest, NoValue) {
    const std::string config = "{ \"name\": \"" + name + "\" }";
    json_t *configJSON = convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_TRUE(result.second.empty());
}

/*
 * Test a general valid conf that doesn't use the mode field
 */
TEST_F(EnvGatewayParserTest, ValidConfWithoutMode) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\"}";
    json_t *configJSON = convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value);
}


/*
 * Test that setting mode to just "set" works as intended
 */
TEST_F(EnvGatewayParserTest, ValidConfModeSet) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"mode\": \"set\" }";
    json_t *configJSON = convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value);
}

TEST_F(EnvGatewayParserTest, BadMode) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"mode\": \"foo\" }";
    json_t *configJSON = convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
}

TEST_F(EnvGatewayParserTest, ModeIsCaseInsensitive) {
    const std::string config1 = "{ \"name\": \"" + name + "\",\
                                   \"value\": \"" + value + "\",\
                                   \"mode\": \"SET\" }";
    json_t *configJSON1 = convertToJSON(config1);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON1, result, store));

    const std::string config2 = "{ \"name\": \"" + name + "\",\
                                   \"value\": \"" + value + "\",\
                                   \"mode\": \"PrEpEnD\" }";
    json_t *configJSON2 = convertToJSON(config2);
    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON2, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value);
}

/*
 * Test that appending to a non-existing var just sets it to the given value
 */
TEST_F(EnvGatewayParserTest, ValidConfAppendEmpty) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"mode\": \"append\" }";
    json_t *configJSON = convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value);
}

TEST_F(EnvGatewayParserTest, ValidConfPrependEmpty) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"mode\": \"prepend\" }";
    json_t *configJSON = convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value);
}

/*
 * Test that appending to an already existing var actually appends the value
 */
TEST_F(EnvGatewayParserTest, ValidConfAppendActuallyAppends) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"mode\": \"append\" }";
    json_t *configJSON = convertToJSON(config);
    store[name] = name;

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, name + value);
}

/*
 * Test that prepending to an already existing var actually prepends the value
 */
TEST_F(EnvGatewayParserTest, ValidConfPrependActuallyPrepends) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"mode\": \"prepend\" }";
    json_t *configJSON = convertToJSON(config);
    store[name] = name;

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value + name);
}

TEST_F(EnvGatewayParserTest, ValidConfPrependSeparatorSeparates) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"separator\": \"" + separator + "\",\
                                  \"mode\": \"prepend\" }";
    json_t *configJSON = convertToJSON(config);
    store[name] = name;

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, value + separator + name);
}

TEST_F(EnvGatewayParserTest, ValidConfAppendSeparatorSeparates) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\",\
                                  \"separator\": \"" + separator + "\",\
                                  \"mode\": \"append\" }";
    json_t *configJSON = convertToJSON(config);
    store[name] = name;

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
    ASSERT_EQ(result.first, name);
    ASSERT_EQ(result.second, name + separator + value);
}

/*
 * Test that setting an already existing var without append fails
 */
TEST_F(EnvGatewayParserTest, SameVarWithoutAppendOrPrependFails) {
    const std::string config = "{ \"name\": \"" + name + "\",\
                                  \"value\": \"" + value + "\"}";
    json_t *configJSON = convertToJSON(config);
    store[name] = value;

    ASSERT_EQ(ReturnCode::FAILURE,
              parser.parseEnvironmentGatewayConfigElement(configJSON, result, store));
}

