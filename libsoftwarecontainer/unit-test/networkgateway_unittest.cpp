
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


#include "gateway/networkgateway.h"
#include "softwarecontainer_test.h"
#include "softwarecontainer-common.h"

class MockNetworkGateway :
    public NetworkGateway
{
public:
    MockNetworkGateway():NetworkGateway() {}

    MOCK_METHOD0(isBridgeAvailable, bool());
};

class NetworkGatewayTest : public SoftwareContainerGatewayTest
{
protected:
    ::testing::NiceMock<MockNetworkGateway> *gw;

    void SetUp() override
    {
        gw = new ::testing::NiceMock<MockNetworkGateway>();
        SoftwareContainerLibTest::SetUp();
    }

    const std::string VALID_FULL_CONFIG =
    "[{"
        "\"type\": \"OUTGOING\","
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"example.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"93.184.216.34/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}, {"
        "\"type\": \"OUTGOING\","
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"example.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"93.184.216.34/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"DROP\""
    "}, {"
        "\"type\": \"OUTGOING\","
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"example.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"93.184.216.34/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"REJECT\""
    "}, {"
        "\"type\": \"INCOMING\","
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"example.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"93.184.216.34/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}, {"
        "\"type\": \"INCOMING\","
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"example.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"93.184.216.34/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"DROP\""
    "}, {"
        "\"type\": \"INCOMING\","
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"example.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"93.184.216.34/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"REJECT\""
    "}]";
};

TEST_F(NetworkGatewayTest, TestNoContainer) {
    ASSERT_FALSE(gw->activate());
    delete gw;
}

/**
 * @brief Test NetworkGateway calls ControllerInterface::systemCall when
 * NetworkGateway::setConfig() has been called
 *
 * The NetworkGateway::setConfig() should try to issue a ifconfig
 * system call inside the container in order to set the containers IP
 * address.
 */
TEST_F(NetworkGatewayTest, TestSetConfig) {
    givenContainerIsSet(gw);

    ASSERT_TRUE(gw->setConfig(VALID_FULL_CONFIG));
}

/*! Test that config entries with no type specified fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigNoType) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}]";

    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with no rules specified fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigNoRules) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"type\": \"OUTGOING\","
        "\"default\": \"ACCEPT\""
    "}]";

    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with no default target specified fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigNoDefaultTarget) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"type\": \"OUTGOING\","
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "]"
    "}]";

    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with type specified as an integer fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigTypeIsInt) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"type\": 123,"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}]";

    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with type specified as a string fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigTypeIsBadString) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"type\": \"otcnm\","
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}]";

    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with type specified as an object fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigTypeIsObject) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"type\": {\"type\": \"INCOMMING\"},"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}]";

    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with type specified as an array fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigTypeIsArray) {
    givenContainerIsSet(gw);
    const std::string config =
    "[{"
        "\"type\": [\"INCOMING\"],"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"ACCEPT\""
    "}]";

    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with rules specified as an object fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigRulesIsObject) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"type\": \"INCOMING\","
        "\"rules\": {"
                     "\"rule1\": { \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "\"rule2\": { \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "\"rule3\": { \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "\"rule4\": { \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "},"
        "\"default\": \"ACCEPT\""
    "}]";
    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with rules specified as a string fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigRulesIsString) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"type\": \"INCOMING\","
        "\"rules\": \""
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "\","
        "\"default\": \"ACCEPT\""
    "}]";
    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with rules specified as an integer fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigRulesIsInteger) {
    givenContainerIsSet(gw);

    const std::string config =
    "[{"
        "\"type\": \"INCOMING\","
        "\"rules\": 123"
        "\"default\": \"ACCEPT\""
    "}]";
    ASSERT_FALSE(gw->setConfig(config));
}

/*! Test that config entries with default specified as a invali target fails gracefully.
 */
TEST_F(NetworkGatewayTest, TestSetConfigDefaultTargetIsInvalid) {
    givenContainerIsSet(gw);
    const std::string config =
    "[{"
        "\"type\": [\"INCOMING\"],"
        "\"rules\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"port\": 80, \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"google.com\", \"port\": \"80-85\", \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"port\": [80, 8080], \"target\": \"ACCEPT\"},"
                     "{ \"host\": \"50.63.202.33/24\", \"target\": \"REJECT\"}"
                 "],"
        "\"default\": \"FOO\""
    "}]";

    ASSERT_FALSE(gw->setConfig(config));
}

/**
 * @brief Test NetworkGateway::activate is successful.
 */
TEST_F(NetworkGatewayTest, TestActivate) {
    givenContainerIsSet(gw);

    ::testing::DefaultValue<bool>::Set(true);
    ASSERT_TRUE(gw->setConfig(VALID_FULL_CONFIG));
    ASSERT_TRUE(gw->activate());
}

/**
 * @brief Test NetworkGateway::activate is successful and that correct command is
 *  issued the second time it is called.
 */
TEST_F(NetworkGatewayTest, TestActivateTwice) {
    givenContainerIsSet(gw);

    ::testing::DefaultValue<bool>::Set(true);
    ASSERT_TRUE(gw->setConfig(VALID_FULL_CONFIG));
    ASSERT_TRUE(gw->activate());
    ASSERT_TRUE(gw->activate());
}

/**
 * @brief Test NetworkGateway::activate is successful but that no network interface
 *  is brought up when the networking config is malformed.
 */
TEST_F(NetworkGatewayTest, TestActivateBadConfig) {
    givenContainerIsSet(gw);
    const std::string config = "[{\"internet-access\": true}]";

    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

/**
 * @brief Test NetworkGateway::activate fails when there is no network bridge setup
 *  on the host.
 */
TEST_F(NetworkGatewayTest, TestActivateNoBridge) {
    givenContainerIsSet(gw);

    ::testing::DefaultValue<bool>::Set(false);
    {
        ::testing::InSequence sequence;
        EXPECT_CALL(*gw, isBridgeAvailable());
    }
    ASSERT_TRUE(gw->setConfig(VALID_FULL_CONFIG));
    ASSERT_FALSE(gw->activate());
}
