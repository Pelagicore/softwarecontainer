/*
 * Copyright (C) 2016 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */


#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <glibmm.h>

#include "gateway.h"
#include "softwarecontainer.h"

/* Mock the PAMAbstractInterface class */
class MockPAMAbstractInterface :
    public PAMAbstractInterface
{
public:
    MOCK_METHOD2(registerClient,
            void(const std::string & cookie, const std::string & appId));

    MOCK_METHOD1(unregisterClient,
            void(const std::string & appId));

    MOCK_METHOD1(updateFinished,
            void(const std::string & appId));
};

class MockGateway :
    public Gateway
{
public:
    MockGateway() :
        Gateway()
    {
    }

    MOCK_METHOD0(id, std::string());
    MOCK_METHOD1(setConfig, bool(const std::string & config));
    MOCK_METHOD0(activate, bool());
    MOCK_METHOD0(teardown, bool());
};

using::testing::InSequence;
using::testing::_;
using::testing::Return;
using::testing::NiceMock;

/*! Test SoftwareContainers interaction with Platform Access Manager
 *
 * The test uses a mock of the abstract interface of PAM and calls
 * metods on SoftwareContainer in the same sequence as the Launcher and
 * PAM would do. The assertions are only on how SoftwareContainer calls
 * PAM in response to these calls. The shutdown phase is not tested since
 * it relies on the controller to be running and catching it's exit signal
 * after calling shutdown. This is tested in the component tests instead.
 */
TEST(SoftwareContainerTest, DISABLED_TestInteractionWithPAM) {
    std::string appId = "the-app-id";
    const std::string cookie = "mycookie";

    MockPAMAbstractInterface pam;
    Glib::RefPtr<Glib::MainLoop> mainloop = Glib::MainLoop::create();
    SoftwareContainer pc(cookie);
    pc.setPAM(pam);

    /* The calls should be made in the specific order as below: */
    {
        InSequence sequence;
        EXPECT_CALL(pam, registerClient(cookie, appId)).Times(1);
        EXPECT_CALL(pam, updateFinished(cookie)).Times(1);
    }

    pc.launch(appId);
    pc.update(std::map<std::string, std::string>({{"", ""}}));
}

/*! Test SoftwareContainer calls Gateway::setConfig and Gateway::activate when
 * SoftwareContainer::update has been called
 *
 * All gateways which have an ID that is found in the configs map passed to
 * SoftwareContainer::update should have their gateways set and be activated by
 * SoftwareContainer. Also checks that Gateway::id is called and sets a return
 * value for SoftwareContainer to use when matching gateway-ID with gateways
 * when configuration is set.
 */
TEST(SoftwareContainerTest, TestCallUpdateShouldSetGatewayConfigsAndActivate) {
    /* Nice mock, i.e. don't warn about uninteresting calls on this mock */
    NiceMock<MockPAMAbstractInterface> pam;
    Glib::RefPtr<Glib::MainLoop> mainloop = Glib::MainLoop::create();

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
    SoftwareContainer pc(cookie);
    pc.setPAM(pam);

    pc.addGateway(gw1);
    pc.addGateway(gw2);
    pc.addGateway(gw3);

    std::map<std::string, std::string> configs
    {{gw1Id, ""}, {gw2Id, ""}, {gw3Id, ""}};

    pc.update(configs);
}
