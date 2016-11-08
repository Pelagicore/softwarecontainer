
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


#include "gateway/networkgatewayparser.h"
#include "softwarecontainer-common.h"
#include "gateway/iptableentry.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <jansson.h>

class NetworkGatewayParserTest : public ::testing::Test
{
protected:
    NetworkGatewayParser networkParser;
};

/*
 * @brief Tests if configuration parsed to rules correctly
 *
 * This tests evaluates following elements parsed correctly
 *      type : INCOMING
 *      default target : DROP
 *      a rule with singular port
 */
TEST_F(NetworkGatewayParserTest, TestInputSingularPort) {

    const std::string config =
    "{"
        "\"type\": \"INCOMING\","
        "\"priority\": 1,"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"}"
                 "],"
        "\"default\": \"DROP\""
    "}";

    IPTableEntry e;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    ASSERT_NE(nullptr, root);
    ASSERT_EQ(ReturnCode::SUCCESS, networkParser.parseNetworkGatewayConfiguration(root, e));
    ASSERT_EQ(IPTableEntry::Target::DROP, e.m_defaultTarget);
    ASSERT_EQ("INPUT", e.m_type);

    ASSERT_EQ("127.0.0.1/16", e.m_rules[0].host);
    ASSERT_EQ(1, e.m_rules[0].ports.any);
    ASSERT_EQ(0, e.m_rules[0].ports.multiport);
    ASSERT_EQ("80", e.m_rules[0].ports.ports);
    ASSERT_EQ(IPTableEntry::Target::ACCEPT, e.m_rules[0].target);
}

/*
 * @brief Tests if configuration parsed to rules correctly
 *
 * This tests evaluates following elements parsed correctly
 *      type : INCOMING
 *      default target : ACCEPT
 *      a rule with port list
 */
TEST_F(NetworkGatewayParserTest, TestInputMultiplePort) {

    const std::string config =
    "{"
        "\"type\": \"INCOMING\","
        "\"priority\": 1,"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}";

    IPTableEntry e;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    ASSERT_NE(nullptr, root);
    ASSERT_EQ(ReturnCode::SUCCESS, networkParser.parseNetworkGatewayConfiguration(root, e));
    ASSERT_EQ(IPTableEntry::Target::ACCEPT, e.m_defaultTarget);
    ASSERT_EQ("INPUT", e.m_type);

    ASSERT_EQ("127.0.0.1/16", e.m_rules[0].host);
    ASSERT_EQ(1, e.m_rules[0].ports.any);
    ASSERT_EQ(1, e.m_rules[0].ports.multiport);
    ASSERT_EQ("80,8080", e.m_rules[0].ports.ports);
    ASSERT_EQ(IPTableEntry::Target::REJECT, e.m_rules[0].target);
}

/*
 * @brief Tests if configuration parsing gives error on wrong default target
 */
TEST_F(NetworkGatewayParserTest, TestInputWrongPolicy) {

    const std::string config =
    "{"
        "\"type\": \"INCOMING\","
        "\"priority\": 1,"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"REJECT\""
    "}";

    IPTableEntry e;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    ASSERT_NE(nullptr, root);
    ASSERT_NE(ReturnCode::SUCCESS, networkParser.parseNetworkGatewayConfiguration(root, e));
}

/*
 * @brief Tests if configuration parsed to rules correctly
 *
 * This tests evaluates following elements parsed correctly
 *      type : OUTGOING
 *      default target : ACCEPT
 *      a rule with port range
 */
TEST_F(NetworkGatewayParserTest, TestOutputMultiplePort) {

    const std::string config =
    "{"
        "\"type\": \"OUTGOING\","
        "\"priority\": 1,"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": \"80:8080\", \"target\": \"DROP\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}";

    IPTableEntry e;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    ASSERT_NE(nullptr, root);
    ASSERT_EQ(ReturnCode::SUCCESS, networkParser.parseNetworkGatewayConfiguration(root, e));
    ASSERT_EQ(IPTableEntry::Target::ACCEPT, e.m_defaultTarget);
    ASSERT_EQ("OUTPUT", e.m_type);

    ASSERT_EQ("127.0.0.1/16", e.m_rules[0].host);
    ASSERT_EQ(1, e.m_rules[0].ports.any);
    ASSERT_EQ(1, e.m_rules[0].ports.multiport);
    ASSERT_EQ("80:8080", e.m_rules[0].ports.ports);
    ASSERT_EQ(IPTableEntry::Target::DROP, e.m_rules[0].target);
}

/*
 * @brief Tests if configuration parsing gives error on wrong target
 */
TEST_F(NetworkGatewayParserTest, TestOutputWrongTarget) {

    const std::string config =
    "{"
        "\"type\": \"OUTGOING\","
        "\"priority\": 1,"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": \"80:8080\", \"target\": \"INJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}";

    IPTableEntry e;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    ASSERT_NE(nullptr, root);
    ASSERT_NE(ReturnCode::SUCCESS, networkParser.parseNetworkGatewayConfiguration(root, e));

}

/*
 * @brief Tests if configuration parsing gives error on no target
 */
TEST_F(NetworkGatewayParserTest, TestOutputNoTarget) {

    const std::string config =
    "{"
        "\"type\": \"OUTGOING\","
        "\"priority\": 1,"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": \"80:8080\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}";

    IPTableEntry e;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    ASSERT_NE(nullptr, root);
    ASSERT_NE(ReturnCode::SUCCESS, networkParser.parseNetworkGatewayConfiguration(root, e));

}

/*
 * @brief Tests if configuration parsing gives error on no host
 */
TEST_F(NetworkGatewayParserTest, TestOutputNoHost) {

    const std::string config =
    "{"
        "\"type\": \"OUTGOING\","
        "\"priority\": 1,"
        "\"rules\": ["
            "{ \"port\": \"80:8080\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}";

    IPTableEntry e;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    ASSERT_NE(nullptr, root);
    ASSERT_NE(ReturnCode::SUCCESS, networkParser.parseNetworkGatewayConfiguration(root, e));

}


/*
 * @brief Tests if configuration parsing does not gives error on no port
 */
TEST_F(NetworkGatewayParserTest, TestOutputNoPort) {

    const std::string config =
    "{"
        "\"type\": \"OUTGOING\","
        "\"priority\": 1,"
        "\"rules\": ["
            "{ \"host\": \"127.0.0.1/16\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}";

    IPTableEntry e;
    json_error_t error;
    json_t *root = json_loads(config.c_str(), 0, &error);

    ASSERT_NE(nullptr, root);
    ASSERT_EQ(ReturnCode::SUCCESS, networkParser.parseNetworkGatewayConfiguration(root, e));

    ASSERT_EQ(0, e.m_rules[0].ports.any);

}
