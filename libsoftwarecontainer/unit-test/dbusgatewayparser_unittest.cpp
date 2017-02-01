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

#include "gateway/dbus/dbusgatewayparser.h"
#include "gateway/dbus/dbusgateway.h"
#include "gateway/gatewayparsererror.h"

#include "gateway_parser_common.h"

using namespace softwarecontainer;

class DBusGatewayParserTest : public GatewayParserCommon<std::string>
{

public:
    DBusGatewayParser parser;
    json_t *sessionConf = json_array();
    json_t *systemConf = json_array();

    size_t empty = 0;

    void SetUp() override
    {
        teardownAtEndOfTest(sessionConf);
        teardownAtEndOfTest(systemConf);
    }

    void testWrongTypeFails(const std::string config) {
        json_t *configJSON = convertToJSON(config);

        // Only works with system config right now.
        EXPECT_THROW(parser.parseDBusConfig(configJSON,
                                            DBusGatewayInstance::SYSTEM_CONFIG,
                                            systemConf),
                     GatewayParserError);

        ASSERT_EQ(json_array_size(sessionConf), empty);
        ASSERT_EQ(json_array_size(systemConf), empty);
    }

    void parse(const std::string &config,
               const bool systemExpectedResult,
               const bool sessionExpectedResult) {
        json_t *configJSON = convertToJSON(config);

        bool systemBusParseResult = parser.parseDBusConfig(configJSON,
                                                           DBusGatewayInstance::SYSTEM_CONFIG,
                                                           systemConf);
        ASSERT_EQ(systemExpectedResult, systemBusParseResult);

        bool sessionBusParseResult = parser.parseDBusConfig(configJSON,
                                                            DBusGatewayInstance::SESSION_CONFIG,
                                                            sessionConf);
        ASSERT_EQ(sessionExpectedResult, sessionBusParseResult);
    }
};

/*
 * Make sure it is possible to config without a system config (but with a session config)
 */
TEST_F(DBusGatewayParserTest, TestNoSystemConf) {
    const std::string config =
        "{"
            "\"" + std::string(DBusGatewayInstance::SESSION_CONFIG) + "\": [{}]"
        "}";
    parse(config, false, true);

    ASSERT_EQ(json_array_size(systemConf), empty);
    ASSERT_NE(json_array_size(sessionConf), empty);
}

/*
 * Make sure it is possible to config without a session config (but with a system config)
 */
TEST_F(DBusGatewayParserTest, TestNoSessionConf) {
    const std::string config =
        "{"
            "\"" + std::string(DBusGatewayInstance::SYSTEM_CONFIG) + "\": [{}]"
        "}";
    parse(config, true, false);

    ASSERT_EQ(json_array_size(sessionConf), empty);
    ASSERT_NE(json_array_size(systemConf), empty);
}

/*
 * Make sure we fail to config when both session and system configs are missing.
 */
TEST_F(DBusGatewayParserTest, TestNoConfAtAll) {
    const std::string config = "{}";
    parse(config, false, false);

    ASSERT_EQ(json_array_size(sessionConf), empty);
    ASSERT_EQ(json_array_size(systemConf), empty);
}

/*
 * Make sure we fail whenever the configs are of wrong types
 */
TEST_F(DBusGatewayParserTest, TestConfigsOfWrongType) {
    const std::string configObject =
        "{"
            "  \"" + std::string(DBusGatewayInstance::SYSTEM_CONFIG) + "\": {}"
        "}";
    testWrongTypeFails(configObject);

    const std::string configString =
        "{"
            "  \"" + std::string(DBusGatewayInstance::SYSTEM_CONFIG) + "\": \"string\""
        "}";
    testWrongTypeFails(configString);

    const std::string configBool =
        "{"
            "  \"" + std::string(DBusGatewayInstance::SYSTEM_CONFIG) + "\": true"
        "}";
    testWrongTypeFails(configBool);
}

/*
 * Make sure we succeed configuration when a valid (although minimal) config is given.
 */
TEST_F(DBusGatewayParserTest, TestFullValidConfig) {
    const std::string config =
        "{"
            "  \"" + std::string(DBusGatewayInstance::SYSTEM_CONFIG) + "\": [{}]"
            ", \"" + std::string(DBusGatewayInstance::SESSION_CONFIG) + "\": [{}]"
        "}";

    parse(config, true, true);

    ASSERT_NE(json_array_size(sessionConf), empty);
    ASSERT_NE(json_array_size(systemConf), empty);
}
