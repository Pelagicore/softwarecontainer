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

#include "gateway/dbus/dbusgatewayparser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class DBusGatewayParserTest : public ::testing::Test
{

public:
    DBusGatewayParser parser;
    json_t *sessionConf = json_array();
    json_t *systemConf = json_array();

    json_error_t err;
    json_t *configJSON;

    size_t empty = 0;

    void convertToJSON(const std::string config)
    {
        configJSON = json_loads(config.c_str(), 0, &err);
        ASSERT_TRUE(configJSON != NULL);
    }

    void TestWrongTypeFails(const std::string config) {
        convertToJSON(config);

        ASSERT_EQ(ReturnCode::FAILURE,
                  parser.parseDBusConfigElement(configJSON, sessionConf, systemConf));
        ASSERT_EQ(json_array_size(sessionConf), empty);
        ASSERT_EQ(json_array_size(systemConf), empty);
    }
};

/*
 * Make sure it is possible to config without a system config (but with a session config)
 */
TEST_F(DBusGatewayParserTest, TestNoSystemConf) {
    const std::string config = 
        "{"
            "\"" + std::string(parser.SESSION_CONFIG) + "\": [{}]"
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseDBusConfigElement(configJSON, sessionConf, systemConf));
    
    ASSERT_EQ(json_array_size(systemConf), empty);
    ASSERT_NE(json_array_size(sessionConf), empty);
}

/*
 * Make sure it is possible to config without a session config (but with a system config)
 */
TEST_F(DBusGatewayParserTest, TestNoSessionConf) {
    const std::string config = 
        "{"
            "\"" + std::string(parser.SYSTEM_CONFIG) + "\": [{}]"
        "}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseDBusConfigElement(configJSON, sessionConf, systemConf));
    
    ASSERT_EQ(json_array_size(sessionConf), empty);
    ASSERT_NE(json_array_size(systemConf), empty);
}

/*
 * Make sure we fail to config when both session and system configs are missing.
 */
TEST_F(DBusGatewayParserTest, TestNoConfAtAll) {
    const std::string config = "{}";
    convertToJSON(config);

    ASSERT_EQ(ReturnCode::FAILURE,
              parser.parseDBusConfigElement(configJSON, sessionConf, systemConf));
    
    ASSERT_EQ(json_array_size(sessionConf), empty);
    ASSERT_EQ(json_array_size(systemConf), empty);
}

/*
 * Make sure we fail whenever the configs are of wrong types
 */
TEST_F(DBusGatewayParserTest, TestConfigsOfWrongType) {
    const std::string configObject = 
        "{"
            "  \"" + std::string(parser.SYSTEM_CONFIG) + "\": {}"
        "}";
    TestWrongTypeFails(configObject);

    const std::string configString = 
        "{"
            "  \"" + std::string(parser.SYSTEM_CONFIG) + "\": \"string\""
        "}";
    TestWrongTypeFails(configString);

    const std::string configBool =
        "{"
            "  \"" + std::string(parser.SYSTEM_CONFIG) + "\": true"
        "}";
    TestWrongTypeFails(configBool);
}

/*
 * Make sure we succeed configuration when a valid (although minimal) config is given.
 */
TEST_F(DBusGatewayParserTest, TestFullValidConfig) {
    const std::string config = 
        "{"
            "  \"" + std::string(parser.SYSTEM_CONFIG) + "\": [{}]"
            ", \"" + std::string(parser.SESSION_CONFIG) + "\": [{}]"
        "}";

    convertToJSON(config);

    ASSERT_EQ(ReturnCode::SUCCESS,
              parser.parseDBusConfigElement(configJSON, sessionConf, systemConf));
    
    ASSERT_NE(json_array_size(sessionConf), empty);
    ASSERT_NE(json_array_size(systemConf), empty);
}
