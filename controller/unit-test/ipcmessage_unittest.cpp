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

/*! Test that Controller::runApp and Controller::killApp are called by IPCMessage
 * when the corresponding messages are passed to IPCMessage
 */
TEST(IPCMessageTest, TestShouldCallRunAppAndKillApp) {
    MockAbstractController controller;
    IPCMessage message(controller);

    std::string runAppCmd("1");
    std::string killAppCmd("2");

    // The calls should be made in the specific order as below:
    {
        InSequence sequence;
        EXPECT_CALL(controller, runApp()).Times(1);
        EXPECT_CALL(controller, killApp()).Times(1);
    }

    message.handleMessage(runAppCmd);
    message.handleMessage(killAppCmd);
}

/*! Test that Controller::systemCall is called with the expected argument
 * by IPCMessage.
 */
TEST(IPCMessageTest, TestShouldCallSystemCallWithExpectedArg) {
    MockAbstractController controller;
    IPCMessage message(controller);

    std::string systemCallCmd("4 this is a system call");

    std::string expectedArgument("this is a system call");
    EXPECT_CALL(controller, systemCall(expectedArgument)).Times(1);

    message.handleMessage(systemCallCmd);
}

/*! Test that Controller::setEnvironmentVariable is called with the expected
 * arguments by IPCMessage.
 */
TEST(IPCMessageTest, TestShouldCallSetEnvironmentVariableWithExpectedArgs) {
    MockAbstractController controller;
    IPCMessage message(controller);

    std::string setEnvironmentVariableCmd("3 THE_VARIABLE this is the value");

    std::string expectedVariable("THE_VARIABLE");
    std::string expectedValue("this is the value");
    EXPECT_CALL(controller, setEnvironmentVariable(expectedVariable, expectedValue)).Times(1);

    message.handleMessage(setEnvironmentVariableCmd);
}

/*! Test that IPCMessage returns the expected value. Valid messages should
 * get 'true' back, and invalid messages should get 'false'
 */
TEST(IPCMessageTest, TestSendShouldReturnExpectedValue) {
    NiceMock<MockAbstractController> controller;
    IPCMessage message(controller);

    std::string validMessage("4 valid message");
    std::string invalidMessage("invalid message");
    std::string killAppMessage("2");

    bool returnVal;

    returnVal = message.handleMessage(validMessage);
    EXPECT_EQ(returnVal, true);

    returnVal = message.handleMessage(invalidMessage);
    EXPECT_EQ(returnVal, false);

    returnVal = message.handleMessage(killAppMessage);
    EXPECT_EQ(returnVal, true);
}
