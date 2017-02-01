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


#include "softwarecontainer-common.h"
#include "gateway/network/networkgatewayparser.h"
#include "gateway/network/iptableentry.h"
#include "gateway_parser_common.h"

#include <jansson.h>

using namespace softwarecontainer;

class NetworkGatewayParserTest : public GatewayParserCommon<std::string>
{
protected:
    NetworkGatewayParser networkParser;
};

/*
 * @brief Tests if configuration parsed to rules correctly
 *
 * This tests evaluates following elements parsed correctly
 *      direction : INCOMING
 *      a rule with singular port
 */
TEST_F(NetworkGatewayParserTest, InputSingularPort) {

    const std::string config =
    "{"
        "\"direction\": \"INCOMING\","
        "\"allow\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"ports\": 80}"
                   "]"
    "}";


    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_TRUE(networkParser.parseNetworkGatewayConfiguration(root, e));
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
 *      direction : INCOMING
 *      a rule with port list
 */
TEST_F(NetworkGatewayParserTest, InputMultiplePort) {

    const std::string config =
    "{"
        "\"direction\": \"INCOMING\","
        "\"allow\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"ports\": [80, 8080]}"
                   "]"
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_TRUE(networkParser.parseNetworkGatewayConfiguration(root, e));
    ASSERT_EQ(IPTableEntry::Target::DROP, e.m_defaultTarget);
    ASSERT_EQ("INPUT", e.m_type);

    ASSERT_EQ("127.0.0.1/16", e.m_rules[0].host);
    ASSERT_EQ(1, e.m_rules[0].ports.any);
    ASSERT_EQ(1, e.m_rules[0].ports.multiport);
    ASSERT_EQ("80,8080", e.m_rules[0].ports.ports);
    ASSERT_EQ(IPTableEntry::Target::ACCEPT, e.m_rules[0].target);
}

/*
 * @brief Tests if configuration parsing fails on port typo
 * This tests evaluates if the test fails when user writes port instead of ports
 */
TEST_F(NetworkGatewayParserTest, PortTypo) {

    const std::string config =
    "{"
        "\"direction\": \"OUTGOING\","
        "\"allow\": "
        "["
           "{ \"host\": \"127.0.0.1/16\", \"port\": \"80:8080\", \"protocols\": \"tcp\"}"
        "]"
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_TRUE(networkParser.parseNetworkGatewayConfiguration(root, e));
    ASSERT_EQ(IPTableEntry::Target::DROP, e.m_defaultTarget);
    ASSERT_EQ("OUTPUT", e.m_type);

    ASSERT_EQ("127.0.0.1/16", e.m_rules[0].host);
    ASSERT_NE(1, e.m_rules[0].ports.any);
    ASSERT_EQ("tcp", e.m_rules[0].protocols[0]);
    ASSERT_EQ(IPTableEntry::Target::ACCEPT, e.m_rules[0].target);
}


/*
 * @brief Tests if configuration parsed to rules correctly
 *
 * This tests evaluates following elements parsed correctly
 *      direction : OUTGOING
 *      a rule with port range and tcp protocol
 */
TEST_F(NetworkGatewayParserTest, OutputMultiplePort) {

    const std::string config =
    "{"
        "\"direction\": \"OUTGOING\","
        "\"allow\": "
        "["
           "{ \"host\": \"127.0.0.1/16\", \"ports\": \"80:8080\", \"protocols\": \"tcp\"}"
        "]"
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_TRUE(networkParser.parseNetworkGatewayConfiguration(root, e));
    ASSERT_EQ(IPTableEntry::Target::DROP, e.m_defaultTarget);
    ASSERT_EQ("OUTPUT", e.m_type);

    ASSERT_EQ("127.0.0.1/16", e.m_rules[0].host);
    ASSERT_EQ(1, e.m_rules[0].ports.any);
    ASSERT_EQ(1, e.m_rules[0].ports.multiport);
    ASSERT_EQ("80:8080", e.m_rules[0].ports.ports);
    ASSERT_EQ("tcp", e.m_rules[0].protocols[0]);
    ASSERT_EQ(IPTableEntry::Target::ACCEPT, e.m_rules[0].target);
}


/*
 * @brief Tests if configuration parsing gives error on no host
 */
TEST_F(NetworkGatewayParserTest, OutputNoHost) {

    const std::string config =
    "{"
        "\"direction\": \"OUTGOING\","
        "\"allow\": "
        "["
           "{ \"ports\": \"80:8080\", \"protocols\": \"tcp\"}"
        "]"
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_FALSE(networkParser.parseNetworkGatewayConfiguration(root, e));
}


/*
 * @brief Tests if configuration parsing does not gives error on no port
 */
TEST_F(NetworkGatewayParserTest, OutputNoPort) {

    const std::string config =
    "{"
        "\"direction\": \"OUTGOING\","
        "\"allow\": "
        "[{\"host\": \"127.0.0.1/16\"}]"
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_TRUE(networkParser.parseNetworkGatewayConfiguration(root, e));

    ASSERT_EQ("127.0.0.1/16", e.m_rules[0].host);
    ASSERT_EQ(0, e.m_rules[0].ports.any);
}

/**
 * @brief Test that config entries with no allow list specified fails gracefully.
 */
TEST_F(NetworkGatewayParserTest, SetConfigNoRules) {

    const std::string config =
    "{"
        "\"direction\": \"OUTGOING\""
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_FALSE(networkParser.parseNetworkGatewayConfiguration(root, e));
}

/**
 * @brief Test that config entries with empty allow list works.
 */
TEST_F(NetworkGatewayParserTest, SetConfigEmptyRules) {

    const std::string config =
    "{"
        "\"direction\": \"OUTGOING\","
        "\"allow\": []"
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_TRUE(networkParser.parseNetworkGatewayConfiguration(root, e));
}

/**
 * @brief Test that config entries with allow list specified as an integer fails gracefully.
 */
TEST_F(NetworkGatewayParserTest, TestSetConfigRulesIsInteger) {

    const std::string config =
    "{"
        "\"direction\": \"INCOMING\","
        "\"allow\": 123"
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_FALSE(networkParser.parseNetworkGatewayConfiguration(root, e));
}


/**
 * @brief Test configuration with multiple protocols and one port
 */
TEST_F(NetworkGatewayParserTest, MultipleProtocols) {

    const std::string config =
    "{"
        "\"direction\": \"INCOMING\","
        "\"allow\": ["
            "{\"host\": \"*\", \"ports\": \"80\", \"protocols\": [\"tcp\", \"udp\"]}"
        "]"
    "}";

    IPTableEntry e;
    json_t *root = convertToJSON(config);

    ASSERT_TRUE(networkParser.parseNetworkGatewayConfiguration(root, e));
    ASSERT_EQ(IPTableEntry::Target::DROP, e.m_defaultTarget);
    ASSERT_EQ("INPUT", e.m_type);

    ASSERT_EQ(1, e.m_rules[0].ports.any);
    ASSERT_EQ("80", e.m_rules[0].ports.ports);
    ASSERT_EQ("tcp", e.m_rules[0].protocols[0]);
    ASSERT_EQ("udp", e.m_rules[0].protocols[1]);
    ASSERT_EQ(IPTableEntry::Target::ACCEPT, e.m_rules[0].target);
}
