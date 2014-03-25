/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "debug.h"
#include "log_console.h"

#include "devicenodegateway.h"
#include "devicenodegateway_unittest_data.h"

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

using ::testing::InSequence;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::StrEq;

TEST(DeviceNodeGatewayTest, TestIdEqualsdevicenode) {
    /* Nice mock, i.e. don't warn about uninteresting calls on this mock */
    NiceMock<MockController> controllerInterface;
    DeviceNodeGateway gw(&controllerInterface);

    ASSERT_STREQ(gw.id().c_str(), "devicenode");
}

TEST(DeviceNodeGatewayTest, TestHasNoEnvironment) {
    /* Nice mock, i.e. don't warn about uninteresting calls on this mock */
    NiceMock<MockController> controllerInterface;
    DeviceNodeGateway gw(&controllerInterface);

    ASSERT_STREQ(gw.environment().c_str(), "");
}

TEST(DeviceNodeGatewayTest, TestCanParseValidConfig) {
    StrictMock<MockController> controllerInterface;
    DeviceNodeGateway gw(&controllerInterface);

    std::string config = "{\"devices\": ["
                         "                  {"
                         "                      \"name\":  \"tty0\","
                         "                      \"major\": \"4\","
                         "                      \"minor\": \"0\","
                         "                      \"mode\":  \"666\""
                         "                  }"
                         "              ]"
                         "}";

    ASSERT_TRUE(gw.setConfig(config));
    EXPECT_CALL(controllerInterface, systemCall(StrEq("mknod tty0 4 0")));
    EXPECT_CALL(controllerInterface, systemCall(StrEq("chmod tty0 666")));
    gw.activate();
}

class DeviceNodeGatewayInvalidConfig : 
    public testing::TestWithParam<testData> {};

INSTANTIATE_TEST_CASE_P (InstantiationName, DeviceNodeGatewayInvalidConfig,
    ::testing::ValuesIn(invalidConfigs));

TEST_P(DeviceNodeGatewayInvalidConfig, handlesInvalidConfig) {
    StrictMock<MockController> controllerInterface;
    DeviceNodeGateway gw(&controllerInterface);

    struct testData config = GetParam();
    ASSERT_FALSE(gw.setConfig(config.data)) << "hej" << config.title;
}
