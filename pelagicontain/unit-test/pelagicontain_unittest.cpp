/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "pelagicontain.h"
#include "pamabstractinterface.h"
#include "mainloopabstractinterface.h"
#include "controllerabstractinterface.h"

/* We use this stub to let Pelagicontain work with it but ignore the calls */
class StubMainloop :
    public MainloopAbstractInterface
{
public:
    virtual void enter()
    {
    }

    virtual void leave()
    {
    }
};

/* We use this stub to let Pelagicontain work with it but ignore the calls */
class StubController :
    public ControllerAbstractInterface
{
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

    virtual bool systemCall(const std::string &cmd)
    {
        return true;
    }
};

/* Mock the PAMAbstractInterface class */
class MockPAMAbstractInterface :
    public PAMAbstractInterface
{
public:
    MOCK_METHOD2(registerClient,
                 void(const std::string &cookie, const std::string &appId));

    MOCK_METHOD1(unregisterClient,
                 void(const std::string &appId));

    MOCK_METHOD1(updateFinished,
                 void(const std::string &appId));
};

class MockGateway :
    public Gateway
{
public:
    virtual std::string environment()
    {
        return "";
    }

    MOCK_METHOD0(id, std::string());
    MOCK_METHOD1(setConfig, bool(const std::string &config));
    MOCK_METHOD0(activate, bool());
    MOCK_METHOD0(teardown, bool());
};

using ::testing::InSequence;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

/*! Test Pelagicontains interaction with Platform Access Manager
 *
 * The test uses a mock of the abstract interface of PAM and calls
 * metods on Pelagicontain in the same sequence as the Launcher and
 * PAM would do. The assertions are only on how Pelagicontain calls
 * PAM in response to these calls.
 */
TEST(PelagicontainTest, TestInteractionWithPAM) {
    std::string appId = "the-app-id";
    const std::string cookie = "mycookie";

    MockPAMAbstractInterface pam;
    StubMainloop mainloop;
    StubController controller;
    Pelagicontain pc(&pam, &mainloop, &controller, cookie);

    /* The calls should be made in the specific order as below: */
    {
        InSequence sequence;
        EXPECT_CALL(pam, registerClient(cookie, appId)).Times(1);
        EXPECT_CALL(pam, updateFinished(cookie)).Times(1);
        EXPECT_CALL(pam, unregisterClient(cookie)).Times(1);
    }

    pc.launch(appId);
    pc.update(std::map<std::string, std::string>({{"", ""}}));
    pc.shutdown();
}

/*! Test Pelagicontain calls Gateway::setConfig and Gateway::activate when
 * Pelagicontain::update has been called
 *
 * All gateways which have an ID that is found in the configs map passed to
 * Pelagicontain::update should have their gateways set and be activated by
 * Pelagicontain. Also checks that Gateway::id is called and sets a return
 * value for Pelagicontain to use when matching gateway-ID with gateways
 * when configuration is set.
 */
TEST(PelagicontainTest, TestCallUpdateShouldSetGatewayConfigsAndActivate) {
    /* Nice mock, i.e. don't warn about uninteresting calls on this mock */
    NiceMock<MockPAMAbstractInterface> pam;
    StubMainloop mainloop;
    StubController controller;

    MockGateway gw1, gw2, gw3;

    std::string gw1Id = "1";
    std::string gw2Id = "2";
    std::string gw3Id = "3";

    {
        InSequence sequence;
        EXPECT_CALL(gw1, id()).Times(1).WillOnce(Return(gw1Id));
        EXPECT_CALL(gw1, setConfig(_)).Times(1);

        EXPECT_CALL(gw2, id()).Times(1).WillOnce(Return(gw2Id));
        EXPECT_CALL(gw2, setConfig(_)).Times(1);

        EXPECT_CALL(gw3, id()).Times(1).WillOnce(Return(gw3Id));
        EXPECT_CALL(gw3, setConfig(_)).Times(1);

        EXPECT_CALL(gw1, activate()).Times(1);
        EXPECT_CALL(gw2, activate()).Times(1);
        EXPECT_CALL(gw3, activate()).Times(1);
    }

    const std::string cookie = "unimportant-cookie";
    Pelagicontain pc(&pam, &mainloop, &controller, cookie);

    pc.addGateway(&gw1);
    pc.addGateway(&gw2);
    pc.addGateway(&gw3);

    std::map<std::string, std::string> configs
    {{gw1Id, ""}, {gw2Id, ""}, {gw3Id, ""}};

    pc.update(configs);
}

/*! Test Pelagicontain calls Gateway::teardown when Pelagicontain::update has
 * been called
 *
 * All gateways that are added on Pelagicontain::initialize should be torn
 * down when Pelagicontain::shutdown has been called.
 */
TEST(PelagicontainTest, TestCallShutdownShouldTearDownGateways) {
    /* "Nice mock", i.e. don't warn about uninteresting calls on this mock */
    NiceMock<MockPAMAbstractInterface> pam;
    StubMainloop mainloop;
    StubController controller;

    /* If we don't use pointers, there will be a crash when Pelagicontain
     * calls delete on the Gateway object
     */
    MockGateway *gw1 = new MockGateway;
    MockGateway *gw2 = new MockGateway;
    MockGateway *gw3 = new MockGateway;

    std::string gw1Id = "1";
    std::string gw2Id = "2";
    std::string gw3Id = "3";

    EXPECT_CALL(*gw1, teardown()).Times(1);
    EXPECT_CALL(*gw2, teardown()).Times(1);
    EXPECT_CALL(*gw3, teardown()).Times(1);

    const std::string cookie = "unimportant-cookie";
    Pelagicontain pc(&pam, &mainloop, &controller, cookie);

    pc.addGateway(gw1);
    pc.addGateway(gw2);
    pc.addGateway(gw3);

    pc.shutdown();
}
