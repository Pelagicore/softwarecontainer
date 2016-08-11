/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gateway_test.h"
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

using::testing::InSequence;
using::testing::_;
using::testing::Return;
using::testing::NiceMock;

class DBusGatewayTest : public GatewayTest
{
public:
    MockDBusGateway *gw;

    void SetUp() override
    {
        gw = new MockDBusGateway(DBusGateway::SessionProxy, m_gatewayDir, m_containerName);
        GatewayTest::SetUp();
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
    lib->addGateway(gw);
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
