/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "debug.h"
#include "log_console.h"

#include "devicenodegateway.h"

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
