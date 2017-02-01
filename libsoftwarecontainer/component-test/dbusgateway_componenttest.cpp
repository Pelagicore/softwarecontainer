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

#include "softwarecontainer_test.h"

#include "gateway/dbus/dbusgatewayinstance.h"
#include "softwarecontainer-common.h"

class MockDBusGatewayInstance : public DBusGatewayInstance
{
public:

    MockDBusGatewayInstance(ProxyType type, const std::string &gatewayDir, const std::string &name):
        DBusGatewayInstance(type, gatewayDir, name)
    {
    }

    MOCK_METHOD2(startDBusProxy, bool(const std::vector<std::string> &commandVec, const std::vector<std::string> &envVec));
    MOCK_METHOD1(testDBusConnection, bool(const std::string &config));

};

using ::testing::_;

class DBusGatewayInstanceTest : public SoftwareContainerGatewayTest
{
public:
    MockDBusGatewayInstance *gw;
    const std::string m_gatewayDir = "/tmp/dbusgateway-unit-test/";
    const std::string m_containerName = "test";

    void SetUp() override
    {
        SoftwareContainerTest::SetUp();
    }

    void setupGateway(DBusGatewayInstance::ProxyType proxyType)
    {
        gw = new MockDBusGatewayInstance(proxyType, m_gatewayDir, m_containerName);
        givenContainerIsSet(gw);
    }

    void testActivate(const std::string &config)
    {
        ::testing::DefaultValue<bool>::Set(true);
        {
            ::testing::InSequence sequence;
            EXPECT_CALL(*gw, startDBusProxy(_, _));
            EXPECT_CALL(*gw, testDBusConnection(_));
        }

        loadConfig(config);

        ASSERT_TRUE(gw->setConfig(jsonConfig));
        ASSERT_TRUE(gw->activate());
    }
};

/*
 * Test that activate works with a proper full session config
 */
TEST_F(DBusGatewayInstanceTest, ActivateSessionBus) {
    setupGateway(DBusGatewayInstance::ProxyType::SessionProxy);
    const std::string config = "[{"
            "\"" + std::string(DBusGatewayInstance::SESSION_CONFIG) + "\": [{"
               "\"direction\": \"*\","
               "\"interface\": \"*\","
               "\"object-path\": \"*\","
               "\"method\": \"*\""
            "}]}]";
    testActivate(config);
}

/*
 * Test that activate works with a proper full system config
 */
TEST_F(DBusGatewayInstanceTest, ActivateSystemBus) {
    setupGateway(DBusGatewayInstance::ProxyType::SystemProxy);
    const std::string config = "[{"
            "\"" + std::string(DBusGatewayInstance::SYSTEM_CONFIG) + "\": [{"
               "\"direction\": \"*\","
               "\"interface\": \"*\","
               "\"object-path\": \"*\","
               "\"method\": \"*\""
            "}]}]";
    testActivate(config);
}
