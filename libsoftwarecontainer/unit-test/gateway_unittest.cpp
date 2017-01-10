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

#include "softwarecontainer-common.h"
#include "gateway/gateway.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace softwarecontainer;

class MockGateway : public Gateway
{
public:
    MockGateway() : Gateway("mock")
    {
    }

    MOCK_METHOD1(readConfigElement, ReturnCode(const json_t *element));
    MOCK_METHOD0(activateGateway, bool());
    MOCK_METHOD0(teardownGateway, bool());
    MOCK_METHOD0(hasContainer, bool());

};

using ::testing::_;

class GatewayTest : public ::testing::Test
{
public:
    ::testing::NiceMock<MockGateway> gw;
    std::string validConf = "[{ }]";
    std::string validEmptyConf = "[]";

    void SetUp() override
    {
        ::testing::DefaultValue<bool>::Set(true);
        ::testing::DefaultValue<ReturnCode>::Set(ReturnCode::SUCCESS);
    }
};

/*
 * Test that neither activate nor teardown works on a gateway that has not been configured.
 */
TEST_F(GatewayTest, ActivateWithoutConfigure) {
    ASSERT_THROW(gw.activate(), GatewayError);
    ASSERT_THROW(gw.teardown(), GatewayError);
}

/*
 * Test that teardown does not work on a gateway that has not been activated.
 */
TEST_F(GatewayTest, TeardownWithoutActivate) {
    ASSERT_TRUE(isSuccess(gw.setConfig(validConf)));
    ASSERT_THROW(gw.teardown(), GatewayError);
}

/*
 * Test that makes sure configs that are not JSON doesn't pass
 */
TEST_F(GatewayTest, ConfigIsNotJSON) {
    const std::string config = "apabepacepa";
    ASSERT_TRUE(isError(gw.setConfig(config)));
    ASSERT_THROW(gw.activate(), GatewayError);
    ASSERT_THROW(gw.teardown(), GatewayError);
}

/*
 * Test that configs that are not JSON arrays doesn't pass
 */
TEST_F(GatewayTest, ConfigIsNotArray) {
    const std::string config = "{ \"key\": \"value\" }";
    ASSERT_TRUE(isError(gw.setConfig(config)));
    ASSERT_THROW(gw.activate(), GatewayError);
    ASSERT_THROW(gw.teardown(), GatewayError);
}

/*
 * Test that empty JSON arrays are not accepted.
 */
TEST_F(GatewayTest, ConfigIsEmpty) {
    ASSERT_TRUE(isError(gw.setConfig(validEmptyConf)));
    ASSERT_THROW(gw.activate(), GatewayError);
    ASSERT_THROW(gw.teardown(), GatewayError);
}

/*
 * Test that array elements that are not objects are not accepted
 */
TEST_F(GatewayTest, ConfigArrayElementsAreNotObjects)
{
    std::vector<std::string> notObjects;
    notObjects.push_back("[ 1, 2, 3 ]");
    notObjects.push_back("[ \"one\", \"two\", \"three\" ]");
    notObjects.push_back("[ true, false ]");
    notObjects.push_back("[ [], [], [] ]");
    notObjects.push_back("[ null ]");
    notObjects.push_back("[ 2.24e12 ]");

    // Array of bad types
    for (std::string notObj : notObjects) {
        ASSERT_TRUE(isError(gw.setConfig(notObj)));
        ASSERT_THROW(gw.activate(), GatewayError);
        ASSERT_THROW(gw.teardown(), GatewayError);
    }

    // Combined arrays of different types
    for (std::string notObjOuter : notObjects) {
        for (std::string notObjInner : notObjects) {
            json_error_t err;
            json_t *arr1 = json_loads(notObjOuter.c_str(), 0, &err);
            json_t *arr2 = json_loads(notObjInner.c_str(), 0, &err);

            json_array_extend(arr1, arr2);
            std::string notObjCombined = json_dumps(arr1, 0);

            ASSERT_TRUE(isError(gw.setConfig(notObjCombined)));
            ASSERT_THROW(gw.activate(), GatewayError);
            ASSERT_THROW(gw.teardown(), GatewayError);

            json_decref(arr1);
            json_decref(arr2);
        }
    }
}

/*
 * Test that it is possible to configure gateways several times
 */
TEST_F(GatewayTest, CanConfigureManyTimes) {
    for (int i = 0; i < 3; i++) {
        ASSERT_TRUE(isSuccess(gw.setConfig(validConf)));
    }
}

/*
 * Test that successful config means that activate can go through,
 * which in turn means that teardown can succeed.
 */
TEST_F(GatewayTest, ConfigEnablesActivateEnablesTeardown) {
    ASSERT_TRUE(isSuccess(gw.setConfig(validConf)));
    ASSERT_TRUE(isSuccess(gw.activate()));
    ASSERT_TRUE(isSuccess(gw.teardown()));
}

/*
 * Test that double activation is not possible
 */
TEST_F(GatewayTest, CantActivateTwice) {
    ASSERT_TRUE(isSuccess(gw.setConfig(validConf)));
    ASSERT_TRUE(isSuccess(gw.activate()));
    ASSERT_THROW(gw.activate(), GatewayError);
}

/*
 * Test that activate throws an exception if there is no container instance
 * set on gateway.
 */
TEST_F(GatewayTest, NoContainerSetMeansActivateThrows) {
    ASSERT_TRUE(isSuccess(gw.setConfig(validConf)));

    // Make the check for a container instance in activate fail
    ::testing::DefaultValue<bool>::Set(false);
    EXPECT_CALL(gw, hasContainer()); // Namely this check

    ASSERT_THROW(gw.activate(), GatewayError);
    ASSERT_THROW(gw.teardown(), GatewayError);
}

/*
 * Test that teardown throws exception if called on a non activated gateway
 */
TEST_F(GatewayTest, FailedActivateMeansTeardownFails) {
    ASSERT_TRUE(isSuccess(gw.setConfig(validConf)));

    // Override the default mock return value set in test base class
    EXPECT_CALL(gw, activateGateway())
        .WillOnce(::testing::Return(false));

    // activate() will fail becuase activateGateway() returned false
    ASSERT_FALSE(isSuccess(gw.activate()));
    // teardown() should throw an exception when in this state
    ASSERT_THROW(gw.teardown(), GatewayError);
}

/*
 * Test that it is not possible to teardown a gateway twice
 */
TEST_F(GatewayTest, CantTeardownTwice) {
    ASSERT_TRUE(isSuccess(gw.setConfig(validConf)));
    ASSERT_TRUE(isSuccess(gw.activate()));

    ASSERT_TRUE(isSuccess(gw.teardown()));
    ASSERT_THROW(gw.teardown(), GatewayError);
}

/*
 * Test that a gateway that has been torn down can be re-configured and re-activated.
 * Basically, successful teardown should mean a return to created state.
 */
TEST_F(GatewayTest, TornDownGatewayCanBeConfigured) {
    for (int i = 0; i < 3; i++) {
        ASSERT_TRUE(isSuccess(gw.setConfig(validConf)));
        ASSERT_TRUE(isSuccess(gw.activate()));
        ASSERT_TRUE(isSuccess(gw.teardown()));
    }
}
