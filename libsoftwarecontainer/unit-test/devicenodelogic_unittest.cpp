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

class DeviceNodeLogicTest : public  ::testing::Test
{
public:
    LOG_DECLARE_CLASS_CONTEXT("TEST", "Tester");
    DeviceNodeLogicTest() { }
    DeviceNodeLogic *dnl;

    void SetUp() override
    {
        dnl = new DeviceNodeLogic();
    }
};

/*
 * Verify whitelisting in the case when mode is set to more permissive
 */
TEST_F(DeviceNodeLogicTest, ChangeModeMorePermissive) {
    const int itemMode = 644;
    const int newMode  = 777;

    const int expectedMode = 777;

    ASSERT_EQ(expectedMode, dnl->calculateDeviceMode(itemMode,newMode));
}

/*
 * Verify whitelisting in the case when mode is set to more permissive
 * for some groups, but not all
 */
TEST_F(DeviceNodeLogicTest, BothChangeAndNoChangeMode1) {
    const int itemMode = 444;
    const int newMode  = 266;

    const int expectedMode = 466;

    ASSERT_EQ(expectedMode, dnl->calculateDeviceMode(itemMode,newMode));
}

/*
 * Verify whitelisting in the case when mode is set to more permissive
 * for some groups, but not all
 */
TEST_F(DeviceNodeLogicTest, BothChangeAndNoChangeMode2) {
    const int itemMode = 444;
    const int newMode  = 67;

    const int expectedMode = 467;

    ASSERT_EQ(expectedMode, dnl->calculateDeviceMode(itemMode,newMode));
}

/*
 * Verify whitelisting in the case when mode is set to less permissive,
 * implying no change will be made to the mode
 */
TEST_F(DeviceNodeLogicTest, NoModeChangeLessPremissive1) {
    const int itemMode = 644;
    const int newMode  = 422;

    const int expectedMode = 644;

    ASSERT_EQ(expectedMode, dnl->calculateDeviceMode(itemMode,newMode));
}

/*
 * Verify whitelisting in the case when mode is set to less permissive,
 * implying no change will be made to the mode
 */
TEST_F(DeviceNodeLogicTest, NoModeChangeLessPremissive2) {
    const int itemMode = 20;
    const int newMode  = 644;

    const int expectedMode = 644;

    ASSERT_EQ(expectedMode, dnl->calculateDeviceMode(itemMode,newMode));
}

/*
 * Verify whitelisting in the case when mode the same as before
 */
TEST_F(DeviceNodeLogicTest, ModeEqual) {
    const int itemMode  = 666;
    const int newMode   = 666;

    const int expectedMode = 666;

    ASSERT_EQ(expectedMode, dnl->calculateDeviceMode(itemMode,newMode));
}

/*
 * Verify updateDeviceList and findDeviceByName functions working as expected
 */
TEST_F(DeviceNodeLogicTest, findDeviceByNameFunction) {
    DeviceNodeParser::Device testDevice{"testDevice", 3, 5, 753};
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    testDevice.name = "anotherTestDevice";
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    testDevice.name = "yetAnotherTestDevice";
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    auto device = dnl->findDeviceByName("anotherTestDevice");
    ASSERT_EQ("anotherTestDevice", device->name);
}

/*
 * Verify updateDeviceList function is failing as expected when user tries to add
 * another device with same name but different major ID
 */
TEST_F(DeviceNodeLogicTest, failureMismatchingDevice) {
    DeviceNodeParser::Device testDevice{"testDevice", 3, 5, 753};
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    testDevice.name  = "testDevice";
    testDevice.major = 4;
    testDevice.mode  = 777;
    ASSERT_EQ(ReturnCode::FAILURE, dnl->updateDeviceList(testDevice));
}

/*
 * Verify updateDeviceList function is failing as expected when user tries to add
 * another device with same name but different minor ID
 */
TEST_F(DeviceNodeLogicTest, failureMismatchingDevice2) {
    DeviceNodeParser::Device testDevice{"testDevice", 3, 5, 753};
    ASSERT_EQ(ReturnCode::SUCCESS, dnl->updateDeviceList(testDevice));

    testDevice.name  = "testDevice";
    testDevice.minor = 44;
    testDevice.mode  = 777;
    ASSERT_EQ(ReturnCode::FAILURE, dnl->updateDeviceList(testDevice));
}
