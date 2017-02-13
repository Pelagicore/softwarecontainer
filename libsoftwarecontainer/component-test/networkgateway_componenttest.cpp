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

#include "gateway/network/networkgateway.h"
#include "softwarecontainer_test.h"
#include "softwarecontainer-common.h"

class MockNetworkGateway :
    public NetworkGateway
{
public:
    MockNetworkGateway(uint32_t containerID,
                       std::shared_ptr<ContainerAbstractInterface> container) :
        NetworkGateway(containerID,
                       std::string(BRIDGE_DEVICE_TESTING),
                       std::string(BRIDGE_IP_TESTING), // bridge ip
                       std::stoi(BRIDGE_NETMASK_BITLENGTH_TESTING), // bridge netmask
                       container)
    {
    }

    MOCK_METHOD0(isBridgeAvailable, bool());
};

class NetworkGatewayTest : public SoftwareContainerGatewayTest
{
protected:
    std::unique_ptr<::testing::NiceMock<MockNetworkGateway>> gw;

    void SetUp() override
    {
        SoftwareContainerGatewayTest::SetUp();
        ::testing::DefaultValue<bool>::Set(true);

        // Randomize container ID
        srand(time(NULL));
        uint32_t containerID = rand() % 100;

        gw = std::unique_ptr<::testing::NiceMock<MockNetworkGateway>>(
            new ::testing::NiceMock<MockNetworkGateway>(containerID, m_container));
    }

    void TearDown() override
    {
        gw.reset();
        SoftwareContainerGatewayTest::TearDown();
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
    loadConfig(VALID_FULL_CONFIG);
    ASSERT_TRUE(gw->setConfig(jsonConfig));

    ASSERT_TRUE(gw->activate());
}

/**
 * @brief Test that setConfig fails on malformed config and that activate throws
 *        an exception when called and gateway is not configured.
 */
TEST_F(NetworkGatewayTest, ActivateBadConfig) {
    const std::string config = "[{\"internet-access\": true}]";
    loadConfig(config);

    ASSERT_FALSE(gw->setConfig(jsonConfig));
    ASSERT_THROW(gw->activate(), GatewayError);
}

/**
 * @brief Test NetworkGateway::activate fails when there is no network bridge setup
 *  on the host.
 */
TEST_F(NetworkGatewayTest, ActivateNoBridge) {
    loadConfig(VALID_FULL_CONFIG);
    ASSERT_TRUE(gw->setConfig(jsonConfig));

    ::testing::DefaultValue<bool>::Set(false);
    EXPECT_CALL(*gw, isBridgeAvailable());

    ASSERT_FALSE(gw->activate());
}
