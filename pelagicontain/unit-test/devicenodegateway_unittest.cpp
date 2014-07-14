/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "log.h"

#include "devicenodegateway.h"
#include "devicenodegateway_unittest_data.h"

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

    virtual bool hasBeenStarted() const
    {
        return true;
    }

    virtual bool initialize()
    {
        return true;
    }

    MOCK_METHOD1(systemCall, bool(const std::string &cmd));
};

using ::testing::InSequence;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::StrEq;
using ::testing::DefaultValue;

TEST(DeviceNodeGatewayTest, TestIdEqualsdevicenode) {
    /* Nice mock, i.e. don't warn about uninteresting calls on this mock */
    NiceMock<MockController> controllerInterface;
    DeviceNodeGateway gw(controllerInterface);

    ASSERT_STREQ(gw.id().c_str(), "devicenode");
}

class DeviceNodeGatewayValidConfig:
    public testing::TestWithParam<testData> {};

INSTANTIATE_TEST_CASE_P(InstantiationName, DeviceNodeGatewayValidConfig,
                        ::testing::ValuesIn(validConfigs));

TEST_P(DeviceNodeGatewayValidConfig, TestCanParseValidConfig) {
    StrictMock<MockController> controllerInterface;
    DeviceNodeGateway gw(controllerInterface);

    struct testData config = GetParam();

    ASSERT_TRUE(gw.setConfig(config.data));

    DefaultValue<bool>::Set(true);
    for (uint i = 0; i < config.names.size(); i++) {
        std::string name = config.names.at(i);
        std::string major = config.majors.at(i);
        std::string minor = config.minors.at(i);
        std::string mode = config.modes.at(i);

        std::string mknodCmd = "mknod " + name + " c " + major + " " + minor;
        std::string chmodCmd = "chmod " + mode + " " + name;

        EXPECT_CALL(controllerInterface, systemCall(StrEq(mknodCmd.c_str())));
        EXPECT_CALL(controllerInterface, systemCall(StrEq(chmodCmd.c_str())));
    }
    gw.activate();
    DefaultValue<bool>::Clear();
}

class DeviceNodeGatewayInvalidConfig:
    public testing::TestWithParam<testData> {};

INSTANTIATE_TEST_CASE_P(InstantiationName, DeviceNodeGatewayInvalidConfig,
                        ::testing::ValuesIn(invalidConfigs));

TEST_P(DeviceNodeGatewayInvalidConfig, handlesInvalidConfig) {
    StrictMock<MockController> controllerInterface;
    DeviceNodeGateway gw(controllerInterface);

    struct testData config = GetParam();
    ASSERT_FALSE(gw.setConfig(config.data));
}
