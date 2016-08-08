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

    void SetUp() override
    {
        gw = std::unique_ptr<MockDBusGateway>(new MockDBusGateway(DBusGateway::SessionProxy,
                                          m_gatewayDir, m_containerName));
        GatewayTest::SetUp();
    }

    void TearDown() override
    {
        GatewayTest::TearDown();
    }

    const std::string m_gatewayDir = "/tmp/dbusgateway-unit-test/";
    const std::string m_containerName = "test";
};

TEST_F(DBusGatewayTest, TestNoContainer) {
    ASSERT_FALSE(gw->activate());
}

TEST_F(DBusGatewayTest, TestSetConfig) {
    givenContainerIsSet();
    const std::string config = "[]";

    ASSERT_TRUE(gw->setConfig(config));
}

TEST_F(DBusGatewayTest, TestActivate) {
    MockDBusGateway mgw(DBusGateway::SessionProxy, m_gatewayDir, m_containerName);
    lib->getSoftwareContainer().addGateway(mgw);
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
        EXPECT_CALL(mgw, startDBusProxy(_, _));
        EXPECT_CALL(mgw, testDBusConnection(_));
    }
    ASSERT_TRUE(mgw.setConfig(config));
    ASSERT_TRUE(mgw.activate());
}
