/*
 * Copyright (C) 2016-2017 Pelagicore AB
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

#include "gateway/pulsegateway.h"

#include <gtest/gtest.h>
#include "gmock/gmock.h"

using namespace softwarecontainer;

class MockPulseGateway :
    public PulseGateway
{
public:
    MockPulseGateway() {}

    MOCK_METHOD0(enablePulseAudio, bool());
};

class PulseGatewayUnitTests : public ::testing::Test
{
public:
    ::testing::NiceMock<MockPulseGateway> pgw;
    void SetUp() override
    {
    }
};

/*
 * Verify updateDeviceList and findDeviceByName functions working as expected
 */
TEST_F(PulseGatewayUnitTests, whitelist) {
    std::string configStr = "{ \"audio\" : true }";
    json_error_t error;
    json_t *root = json_loads(configStr.c_str(), 0, &error);

    ASSERT_TRUE(pgw.readConfigElement(root));

    std::string configFalseStr = "{ \"audio\" : false }";
    root = json_loads(configFalseStr.c_str(), 0, &error);

    ASSERT_TRUE(pgw.readConfigElement(root));

    // Set the return value for the mocked method
    ::testing::DefaultValue<bool>::Set(true);
    EXPECT_CALL(pgw, enablePulseAudio());

    ASSERT_TRUE(pgw.activateGateway());
}
