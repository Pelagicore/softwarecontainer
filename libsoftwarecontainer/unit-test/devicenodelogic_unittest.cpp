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

//#include "softwarecontainer_test.h"
#include "gateway/devicenode/devicenodelogic.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>
#include <libgen.h>

struct testModes
{
    int itemMode;
    int newMode;
    int expectedMode;
};

class DeviceModeTests : public  ::testing::TestWithParam<testModes>
{
public:
    DeviceNodeLogic *dnl;
    testModes testparams;

    void SetUp() override
    {
        dnl = new DeviceNodeLogic();
        testparams = GetParam();
    }
};

/*
 * This data is fed to the DeviceMode tests
 */
INSTANTIATE_TEST_CASE_P(DeviceModeParameters, DeviceModeTests, ::testing::Values(
        testModes{644, 777 , 777},
        testModes{444, 266 ,466},
        testModes{444, 67  ,467},
        testModes{644, 422 ,644},
        testModes{20,  644 ,644},
        testModes{666, 666 ,666}
));


/*
 * Verify whitelisting in the case when mode is set to more permissive
 */
TEST_P(DeviceModeTests, UpdateDeviceMode) {
    ASSERT_EQ(testparams.expectedMode,
            dnl->calculateDeviceMode(testparams.itemMode,testparams.newMode));
}

class DeviceModeLogicFunctionsTests : public ::testing::Test
{
public:
    DeviceModeLogicFunctionsTests() { };
    DeviceNodeLogic *dnl;

    void SetUp() override
    {
        dnl = new DeviceNodeLogic();
    }
};

/*
 * Verify updateDeviceList and findDeviceByName functions working as expected
 */
TEST_F(DeviceModeLogicFunctionsTests, findDeviceByNameFunction) {
    DeviceNodeParser::Device testDevice{"testDevice", 3, 5, 753};
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    testDevice.name = "anotherTestDevice";
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    testDevice.name = "yetAnotherTestDevice";
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    auto device = dnl->findDeviceByName("anotherTestDevice");
    ASSERT_EQ("anotherTestDevice", device->name);
}


class UpdateDeviceListFailureTests : public  ::testing::TestWithParam<DeviceNodeParser::Device>
{
public:
    DeviceNodeLogic *dnl;
    DeviceNodeParser::Device testparams;

    void SetUp() override
    {
        dnl = new DeviceNodeLogic();
        testparams = GetParam();
    }
};

/*
 * This data is fed to the DeviceMode tests
 */
INSTANTIATE_TEST_CASE_P(DeviceParameters, UpdateDeviceListFailureTests, ::testing::Values(
        DeviceNodeParser::Device{"testDevice", 4, 5, 777},
        DeviceNodeParser::Device{"testDevice", 3, 46, 666},
        DeviceNodeParser::Device{"testDevice", 4, 712, 555}
));

/*
 * Verify updateDeviceList function is failing as expected when user tries to add
 * another device with same name but different major ID
 */
TEST_P(UpdateDeviceListFailureTests, failureMismatchingDevice) {
    DeviceNodeParser::Device testDevice{"testDevice", 3, 5, 753};
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    testDevice.name  = testparams.name;
    testDevice.major = testparams.major;
    testDevice.minor = testparams.minor;
    testDevice.mode  = testparams.mode;
    ASSERT_EQ(ReturnCode::FAILURE, dnl->updateDeviceList(testDevice));
}
