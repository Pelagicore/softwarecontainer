
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

#include "pulsegateway.h"
#include "pulsegateway_unittest_data.h"
#include "generators.h"
#include "softwarecontainer-common.h"

using namespace softwarecontainer;

class PulseMockController
{
public:
    virtual ~PulseMockController()
    {
    }

    virtual bool shutdown()
    {
        return true;
    }

    virtual bool initialize()
    {
        return true;
    }

    MOCK_METHOD1(systemCall,
            ReturnCode(const std::string & cmd));

    MOCK_METHOD2(setEnvironmentVariable,
            ReturnCode(const std::string & variable, const std::string & value));
};


class MockSystemcallInterface :
    public SystemcallAbstractInterface
{
public:
    virtual ~MockSystemcallInterface()
    {
    }

    virtual bool makeCall(const std::string &cmd, int &exitCode)
    {
        exitCode = 0;
        return true;
    }

    MOCK_METHOD1(makeCall, bool(const std::string & cmd));

    MOCK_METHOD3(makePopenCall, pid_t(const std::string & command, int *infp, int *outfp));

    MOCK_METHOD3(makePcloseCall, bool(pid_t pid, int infp, int outfp));

};

using::testing::DefaultValue;
using::testing::InSequence;
using::testing::_;
using::testing::Return;
using::testing::NiceMock;
using::testing::StrictMock;
using::testing::StrEq;
using::testing::A;

class PulseGatewayTest :
    public::testing::Test
{
protected:
    virtual void SetUp()
    {
        DefaultValue<bool>::Set(true);

        // ContainerName and gatewayDir must be passed to constructor
        containerName = "fake-containerName";
        containerRoot = "/fake-directory/";

        containerDir = containerRoot + "/" + containerName;
        gatewayDir = containerDir + "/gateways";
    }

    virtual void TearDown()
    {
        using::testing::DefaultValue;
        DefaultValue<bool>::Clear();
    }

    NiceMock<PulseMockController> controllerInterface;
    std::string containerName;
    std::string gatewayDir;
    std::string containerRoot;
    std::string containerDir;
};

/*! Test that id is correct ("pulseaudio")
 */
TEST_F(PulseGatewayTest, TestIdEqualspulseaudio) {
    PulseGateway gw(gatewayDir, containerName);
    ASSERT_STREQ(gw.id().c_str(), "pulseaudio");
}

/*! Test well-formed configuration that enables the gateway.
 *
 *  Tests with configurations that are syntactically correct, i.e. not
 *  causing errors, and that enables audio.
 */
class PulseGatewayValidConfig :
    public testing::TestWithParam<pulseTestData>
{
};

INSTANTIATE_TEST_CASE_P(InstantiationName, PulseGatewayValidConfig,
        ::testing::ValuesIn(validConfigs));

TEST_P(PulseGatewayValidConfig, DISABLED_TestCanParseValidConfig) {
    StrictMock<PulseMockController> controllerInterface;
    PulseGateway gw("fake-gatewayDir", "fake-containerName");

    struct pulseTestData config = GetParam();

    DefaultValue<bool>::Set(true);
    EXPECT_CALL(controllerInterface, setEnvironmentVariable(
                A<const std::string &>(),
                A<const std::string &>()));

    bool success = gw.setConfig(config.data);
    DefaultValue<bool>::Clear();
    ASSERT_TRUE(success);

    success = gw.activate();
    ASSERT_TRUE(success);

    ASSERT_TRUE(gw.teardown());
}


/*! Test malformed configuration that disables the gateway.
 *
 *  Tests with configurations that are syntactically incorrect, i.e.
 *  causing parsing errors.
 */
class PulseGatewayInvalidConfig :
    public testing::TestWithParam<pulseTestData>
{
};

INSTANTIATE_TEST_CASE_P(InstantiationName, PulseGatewayInvalidConfig,
        ::testing::ValuesIn(invalidConfigs));

TEST_P(PulseGatewayInvalidConfig, TestCanParseInvalidConfig) {
    StrictMock<PulseMockController> controllerInterface;
    PulseGateway gw("fake-gatewayDir", "fake-containerName");

    struct pulseTestData config = GetParam();

    bool success = gw.setConfig(config.data);
    ASSERT_TRUE(!success);
}

/*! Test well-formed configuration that disables the gateway.
 *
 *  Tests with configurations that are syntactically correct, i.e. not
 *  causing errors, but that disables audio.
 */
class PulseGatewayDisablingConfig :
    public testing::TestWithParam<pulseTestData>
{
};

INSTANTIATE_TEST_CASE_P(InstantiationName, PulseGatewayDisablingConfig,
        ::testing::ValuesIn(disablingConfigs));

TEST_P(PulseGatewayDisablingConfig, TestCanParseDisablingConfig) {
    StrictMock<PulseMockController> controllerInterface;
    PulseGateway gw("fake-gatewayDir", "fake-containerName");

    struct pulseTestData config = GetParam();

    EXPECT_CALL(controllerInterface, setEnvironmentVariable(_, _)).Times(0);

    bool success = gw.setConfig(config.data);
    ASSERT_TRUE(success);
}
