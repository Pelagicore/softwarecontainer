
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


#include "softwarecontainer_test.h"
#include "gateway/dbusgateway.h"
#include "softwarecontainer-common.h"

class MockDBusGateway : public DBusGateway
{
public:

    MockDBusGateway(ProxyType type, const std::string &gatewayDir, const std::string &name):
        DBusGateway(type, gatewayDir, name)
    {
    }

    MOCK_METHOD2(startDBusProxy, bool(const std::vector<std::string> &commandVec, const std::vector<std::string> &envVec));
    MOCK_METHOD1(testDBusConnection, bool(const std::string &config));

};

using::testing::_;

class DBusGatewayTest : public SoftwareContainerGatewayTest
{
public:
    MockDBusGateway *gw;

    void SetUp() override
    {
        gw = new MockDBusGateway(DBusGateway::SessionProxy, m_gatewayDir, m_containerName);
        SoftwareContainerLibTest::SetUp();
    }

    const std::string m_gatewayDir = "/tmp/dbusgateway-unit-test/";
    const std::string m_containerName = "test";
};

TEST_F(DBusGatewayTest, TestNoContainer) {
    ASSERT_FALSE(gw->activate());
    delete gw;
}

TEST_F(DBusGatewayTest, TestSetConfig) {
    givenContainerIsSet(gw);
    const std::string config = "[]";

    ASSERT_TRUE(gw->setConfig(config));
}

TEST_F(DBusGatewayTest, TestActivate) {
    givenContainerIsSet(gw);
    const std::string config = "[{"
            "\"dbus-gateway-config-session\": [{"
               "\"direction\": \"*\","
               "\"interface\": \"*\","
               "\"object-path\": \"*\","
               "\"method\": \"*\""
            "}]}]";

    ::testing::DefaultValue<bool>::Set(true);
    {
        ::testing::InSequence sequence;
        EXPECT_CALL(*gw, startDBusProxy(_, _));
        EXPECT_CALL(*gw, testDBusConnection(_));
    }
    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());
}
