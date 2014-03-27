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

class MockController :
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


class SystemCallInterfaceStub :
    public SystemCallAbstractInterface
{
public:

    virtual bool makeCall(const std::string &cmd)
    {
	return true;
    }

    virtual bool makeCall(const std::string &cmd, int &exitCode)
    {
	exitCode = 0;
	return true;
    }

};

using ::testing::InSequence;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;


/*! Test NetworkGateway calls ControllerInterface::systemCall when
 * NetworkGateway::setConfig() has been called
 *
 * The NetworkGateway::setConfig() should try to issue a ifconfig
 * system call inside the container in order to set the containers IP
 * address.
 */
TEST(NetworkGatewayTest, TestSetConfig) {

    std::string config = "{\"internet-access\": \"false\", \"gateway\":\"\"}";
    /* Nice mock, i.e. don't warn about uninteresting calls on this mock */
    NiceMock<MockController> controllerInterface;
    SystemCallInterfaceStub systemCallInterface;
    NetworkGateway gw(&controllerInterface, &systemCallInterface);

    bool success = gw.setConfig(config);
    ASSERT_TRUE(success);
}

/*! Test NetworkGateway::activate is successful.
 */
TEST(NetworkGatewayTest, TestActivate) {
    std::string config = "{\"internet-access\": \"true\", \"gateway\":\"10.0.3.1\"}";
    NiceMock<MockController> controllerInterface;
    SystemCallInterfaceStub systemCallInterface;
    NetworkGateway gw(&controllerInterface, &systemCallInterface);
    ASSERT_TRUE(gw.setConfig(config));

    std::string ip = gw.ip();

    std::string cmd_1 = "ifconfig eth0 " + ip + " netmask 255.255.255.0 up";
    std::string cmd_2 = "route add default gw 10.0.3.1";

    {
	InSequence sequence;
	EXPECT_CALL(controllerInterface, systemCall(cmd_1)).Times(1);
	EXPECT_CALL(controllerInterface, systemCall(cmd_2)).Times(1);
    }

    bool success = gw.activate();
    ASSERT_TRUE(success);
}

/*! Test NetworkGateway::activate is successful.
 */
TEST(NetworkGatewayTest, TestActivateTwice) {
    std::string config = "{\"internet-access\": \"true\", \"gateway\":\"10.0.3.1\"}";
    NiceMock<MockController> controllerInterface;
    SystemCallInterfaceStub systemCallInterface;
    NetworkGateway gw(&controllerInterface, &systemCallInterface);
    ASSERT_TRUE(gw.setConfig(config));

    std::string ip = gw.ip();

    std::string cmd_1 = "ifconfig eth0 " + ip + " netmask 255.255.255.0 up";
    std::string cmd_2 = "route add default gw 10.0.3.1";
    std::string cmd_3 = "ifconfig eth0 up";

    {
	InSequence sequence;
	EXPECT_CALL(controllerInterface, systemCall(cmd_1)).Times(1);
	EXPECT_CALL(controllerInterface, systemCall(cmd_2)).Times(1);
    }

    bool success = gw.activate();
    ASSERT_TRUE(success);

    {
	InSequence sequence;
	EXPECT_CALL(controllerInterface, systemCall(cmd_3)).Times(1);
	EXPECT_CALL(controllerInterface, systemCall(cmd_2)).Times(1);
    }

    success = gw.activate();
    ASSERT_TRUE(success);

}

/*! Test NetworkGateway::activate is successful.
 */
TEST(NetworkGatewayTest, TestActivateNoConfig) {
    NiceMock<MockController> controllerInterface;
    SystemCallInterfaceStub systemCallInterface;
    NetworkGateway gw(&controllerInterface, &systemCallInterface);

    std::string cmd_1 = "ifconfig eth0 down";

    {
	InSequence sequence;
	EXPECT_CALL(controllerInterface, systemCall(cmd_1)).Times(1);
    }

    bool success = gw.activate();
    ASSERT_TRUE(success);
}

/*! Test NetworkGateway::activate is successful.
 */
TEST(NetworkGatewayTest, TestActivateBadConfig) {
    std::string config = "{\"internet-access\": \"true\"}";
    NiceMock<MockController> controllerInterface;
    SystemCallInterfaceStub systemCallInterface;
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
