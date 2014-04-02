/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "networkgateway.h"
#include "debug.h"
#include "log_console.h"

using namespace pelagicore;

class MockController:
    public ControllerAbstractInterface
{
public:

    virtual bool startApp()
    {
        return true;
    }

    virtual bool shutdown()
    {
        return true;
    }

    virtual bool setEnvironmentVariable(const std::string &variable,
                                        const std::string &value)
    {
        return true;
    }

    MOCK_METHOD1(systemCall,
                 bool(const std::string &cmd));
};


class MockSystemcallInterface:
    public SystemcallAbstractInterface
{
public:
    virtual bool makeCall(const std::string &cmd, int &exitCode)
    {
        exitCode = 0;
        return true;
    }

    MOCK_METHOD1(makeCall, bool(const std::string &cmd));

};

using ::testing::DefaultValue;
using ::testing::InSequence;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class NetworkGatewayTest:
    public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        DefaultValue<bool>::Set(true);
    }

    virtual void TearDown()
    {
        using ::testing::DefaultValue;
        DefaultValue<bool>::Clear();
    }

    NiceMock<MockController> controllerInterface;
    NiceMock<MockSystemcallInterface> systemCallInterface;
};

/*! Test NetworkGateway calls ControllerInterface::systemCall when
 * NetworkGateway::setConfig() has been called
 *
 * The NetworkGateway::setConfig() should try to issue a ifconfig
 * system call inside the container in order to set the containers IP
 * address.
 */
TEST_F(NetworkGatewayTest, TestSetConfig) {

    std::string config = "{\"internet-access\": \"false\", \"gateway\":\"\"}";
    NetworkGateway gw(&controllerInterface, &systemCallInterface);

    bool success = gw.setConfig(config);
    ASSERT_TRUE(success);
}

/*! Test NetworkGateway::activate is successful.
 */
TEST_F(NetworkGatewayTest, TestActivate) {
    std::string config = "{\"internet-access\": \"true\", \"gateway\":\"10.0.3.1\"}";
    NetworkGateway gw(&controllerInterface, &systemCallInterface);

    ASSERT_TRUE(gw.setConfig(config));

    std::string cmd_0 = "ifconfig | grep -C 2 \"container-br0\" | grep -q \"10.0.3.1\"";
    std::string cmd_1 = "route add default gw 10.0.3.1";

    {
        InSequence sequence;
        EXPECT_CALL(systemCallInterface, makeCall(cmd_0)).Times(1);
        EXPECT_CALL(controllerInterface, systemCall(_)).Times(1);
        EXPECT_CALL(controllerInterface, systemCall(cmd_1)).Times(1);
    }

    bool success = gw.activate();
    ASSERT_TRUE(success);
}

/*! Test NetworkGateway::activate is successful and that correct command is
 *  issued the second time it is called.
 */
TEST_F(NetworkGatewayTest, TestActivateTwice) {
    std::string config = "{\"internet-access\": \"true\", \"gateway\":\"10.0.3.1\"}";
    NetworkGateway gw(&controllerInterface, &systemCallInterface);

    ASSERT_TRUE(gw.setConfig(config));

    std::string cmd_0 = "ifconfig | grep -C 2 \"container-br0\" | grep -q \"10.0.3.1\"";
    std::string cmd_1 = "route add default gw 10.0.3.1";
    std::string cmd_2 = "ifconfig eth0 up";

    {
        InSequence sequence;
        EXPECT_CALL(systemCallInterface, makeCall(cmd_0)).Times(1);
        EXPECT_CALL(controllerInterface, systemCall(_)).Times(1);
        EXPECT_CALL(controllerInterface, systemCall(cmd_1)).Times(1);
    }

    bool success = gw.activate();
    ASSERT_TRUE(success);

    {
        InSequence sequence;
        EXPECT_CALL(systemCallInterface, makeCall(cmd_0)).Times(1);
        EXPECT_CALL(controllerInterface, systemCall(cmd_2)).Times(1);
        EXPECT_CALL(controllerInterface, systemCall(cmd_1)).Times(1);
    }

    success = gw.activate();
    ASSERT_TRUE(success);

}

/*! Test NetworkGateway::activate is successful but that no network interface
 *  is brought up when there is no config for networking.
 */
TEST_F(NetworkGatewayTest, TestActivateNoConfig) {
    NetworkGateway gw(&controllerInterface, &systemCallInterface);

    std::string cmd_1 = "ifconfig eth0 down";

    {
        InSequence sequence;
        EXPECT_CALL(controllerInterface, systemCall(cmd_1)).Times(1);
    }

    bool success = gw.activate();
    ASSERT_TRUE(success);
}

/*! Test NetworkGateway::activate is successful but that no network interface
 *  is brought up when the networking config is malformed.
 */
TEST_F(NetworkGatewayTest, TestActivateBadConfig) {
    std::string config = "{\"internet-access\": \"true\"}";
    NetworkGateway gw(&controllerInterface, &systemCallInterface);

    ASSERT_TRUE(gw.setConfig(config));

    std::string cmd_1 = "ifconfig eth0 down";

    {
        InSequence sequence;
        EXPECT_CALL(controllerInterface, systemCall(cmd_1)).Times(1);
    }

    bool success = gw.activate();
    ASSERT_TRUE(success);
}

/*! Test NetworkGateway::activate fails when there is no network bridge setup
 *  on the host.
 */
TEST_F(NetworkGatewayTest, TestActivateNoBridge) {
    std::string config = "{\"internet-access\": \"true\", \"gateway\":\"10.0.3.1\"}";
    NetworkGateway gw(&controllerInterface, &systemCallInterface);

    ASSERT_TRUE(gw.setConfig(config));

    DefaultValue<bool>::Set(false);

    bool success = gw.activate();
    EXPECT_CALL(systemCallInterface, makeCall(_)).Times(0);
    ASSERT_TRUE(!success);

    DefaultValue<bool>::Clear();
}
