
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

#include "gateway/network/networkgateway.h"
#include "softwarecontainer_test.h"
#include "softwarecontainer-common.h"

class MockNetworkGateway :
    public NetworkGateway
{
public:
    MockNetworkGateway():NetworkGateway(11, "10.0.3.1", 16) {}

    MOCK_METHOD0(isBridgeAvailable, ReturnCode());
};

class NetworkGatewayTest : public SoftwareContainerGatewayTest
{
protected:
    ::testing::NiceMock<MockNetworkGateway> *gw;

    void SetUp() override
    {
        gw = new ::testing::NiceMock<MockNetworkGateway>();
        SoftwareContainerTest::SetUp();
        ::testing::DefaultValue<ReturnCode>::Set(ReturnCode::SUCCESS);
    }

    const std::string VALID_FULL_CONFIG =
    "[{"
        "\"direction\": \"OUTGOING\","
        "\"allow\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"ports\": 80},"
                     "{ \"host\": \"example.com\", \"ports\": \"80:85\", \"protocols\": \"tcp\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"ports\": [80, 8080], \"protocols\": \"udp\"},"
                     "{ \"host\": \"93.184.216.34/24\"}"
                 "]"
    "},"
    "{"
        "\"direction\": \"OUTGOING\","
        "\"allow\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"ports\": 80},"
                     "{ \"host\": \"example.com\", \"ports\": \"80:85\", \"protocols\": \"tcp\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"ports\": [80, 8080]}"
                   "]"
    "},"
    "{"
        "\"direction\": \"INCOMING\","
        "\"allow\": ["
                     "{ \"host\": \"127.0.0.1/16\", \"ports\": 80, \"protocols\": [\"icmp\", \"udp\"]},"
                     "{ \"host\": \"example.com\", \"ports\": \"80:85\", \"protocols\": \"tcp\"},"
                     "{ \"host\": \"127.0.0.1/16\", \"ports\": [80, 8080]}"
                   "]"
     "}]";
};

/**
 * @brief Test NetworkGateway::activate is successful.
 */
TEST_F(NetworkGatewayTest, Activate) {
    givenContainerIsSet(gw);
    ASSERT_TRUE(gw->setConfig(VALID_FULL_CONFIG));
    ASSERT_TRUE(gw->activate());
}

/**

 * @brief Test NetworkGateway::activate is successful but that no network interface
 *  is brought up when the networking config is malformed.
 */
TEST_F(NetworkGatewayTest, ActivateBadConfig) {
    givenContainerIsSet(gw);
    const std::string config = "[{\"internet-access\": true}]";

    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

/**
 * @brief Test NetworkGateway::activate fails when there is no network bridge setup
 *  on the host.
 */
TEST_F(NetworkGatewayTest, ActivateNoBridge) {
    givenContainerIsSet(gw);

    ::testing::DefaultValue<ReturnCode>::Set(ReturnCode::FAILURE);
        EXPECT_CALL(*gw, isBridgeAvailable());

    ASSERT_TRUE(gw->setConfig(VALID_FULL_CONFIG));
    ASSERT_FALSE(gw->activate());
}
