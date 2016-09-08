
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

#include "softwarecontainer-common.h"

#include "devicenodegateway.h"
#include "devicenodegateway_unittest_data.h"

using namespace softwarecontainer;

class MockController
{
public:
    virtual bool shutdown()
    {
        return true;
    }

    virtual ReturnCode setEnvironmentVariable(const std::string &variable,
            const std::string &value)
    {
        return ReturnCode::SUCCESS;
    }

    virtual bool initialize()
    {
        return true;
    }

    MOCK_METHOD1(systemCall, ReturnCode(const std::string & cmd));
};

using::testing::InSequence;
using::testing::_;
using::testing::Return;
using::testing::NiceMock;
using::testing::StrictMock;
using::testing::StrEq;
using::testing::DefaultValue;

TEST(DeviceNodeGatewayTest, TestIdEqualsdevicenode) {
    /* Nice mock, i.e. don't warn about uninteresting calls on this mock */
    NiceMock<MockController> controllerInterface;
    DeviceNodeGateway gw;

    ASSERT_STREQ(gw.id().c_str(), "devicenode");
}

class DeviceNodeGatewayValidConfig :
    public testing::TestWithParam<testData>
{
};

INSTANTIATE_TEST_CASE_P(InstantiationName, DeviceNodeGatewayValidConfig,
        ::testing::ValuesIn(validConfigs));

TEST_P(DeviceNodeGatewayValidConfig, TestCanParseValidConfig) {
    StrictMock<MockController> controllerInterface;
    DeviceNodeGateway gw;

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

class DeviceNodeGatewayInvalidConfig :
    public testing::TestWithParam<testData>
{
};

INSTANTIATE_TEST_CASE_P(InstantiationName, DeviceNodeGatewayInvalidConfig,
        ::testing::ValuesIn(invalidConfigs));

TEST_P(DeviceNodeGatewayInvalidConfig, handlesInvalidConfig) {
    StrictMock<MockController> controllerInterface;
    DeviceNodeGateway gw;

    struct testData config = GetParam();
    ASSERT_FALSE(gw.setConfig(config.data));
}
