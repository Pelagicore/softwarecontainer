/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <fcntl.h>

#include "abstractcontroller.h"
#include "ipcmessage.h"

class MockAbstractController :
    public AbstractController
{
public:
    MOCK_METHOD0(runApp, int());
    MOCK_METHOD0(killApp, void());
    MOCK_METHOD2(setEnvironmentVariable, void(const std::string &, const std::string &));
    MOCK_METHOD1(systemCall, void(const std::string &));
};

using ::testing::InSequence;
using ::testing::Return;
using ::testing::NiceMock;

TEST(IPCMessageTest, TestShouldCallRunAppAndKillApp) {
    MockAbstractController controller;
    IPCMessage message(&controller);

    std::string runAppCmd("1");
    std::string killAppCmd("2");

    // The calls should be made in the specific order as below:
    {
        InSequence sequence;
        EXPECT_CALL(controller, runApp()).Times(1);
        EXPECT_CALL(controller, killApp()).Times(1);
    }

    int status;
    message.send(runAppCmd, &status);
    message.send(killAppCmd, &status);
}

TEST(IPCMessageTest, TestShouldCallSystemCallWithExpectedArg) {
    MockAbstractController controller;
    IPCMessage message(&controller);

    std::string systemCallCmd("4 this is a system call");

    std::string expectedArgument("this is a system call");
    EXPECT_CALL(controller, systemCall(expectedArgument)).Times(1);

    int status;
    message.send(systemCallCmd, &status);
}

TEST(IPCMessageTest, TestShouldCallSetEnvironmentVariableWithExpectedArgs) {
    MockAbstractController controller;
    IPCMessage message(&controller);

    std::string setEnvironmentVariableCmd("3 THE_VARIABLE this is the value");

    std::string expectedVariable("THE_VARIABLE");
    std::string expectedValue("this is the value");
    EXPECT_CALL(controller, setEnvironmentVariable(expectedVariable, expectedValue)).Times(1);

    int status;
    message.send(setEnvironmentVariableCmd, &status);
}

TEST(IPCMessageTest, TestShouldSetErrorFlagAsExpected) {
    NiceMock<MockAbstractController> controller;
    IPCMessage message(&controller);

    int status = 123;
    message.send(std::string("4 valid message"), &status);
    EXPECT_EQ(status, 0);

    status = 123;
    message.send(std::string("invalid message"), &status);
    EXPECT_EQ(status, -1);
}

TEST(IPCMessageTest, TestSendShouldReturnExpectedValue) {
    NiceMock<MockAbstractController> controller;
    IPCMessage message(&controller);

    std::string validMessage("4 valid message");
    std::string invalidMessage("invalid message");
    std::string killAppMessage("2");

    int status;
    bool returnVal;

    returnVal = message.send(validMessage, &status);
    EXPECT_EQ(returnVal, true);

    returnVal = message.send(invalidMessage, &status);
    EXPECT_EQ(returnVal, true);

    returnVal = message.send(killAppMessage, &status);
    EXPECT_EQ(returnVal, false);
}
