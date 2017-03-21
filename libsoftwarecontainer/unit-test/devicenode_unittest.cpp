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

#include "gateway/devicenode/devicenode.h"
#include "gateway/devicenode/devicenodelogic.h"
#include "gateway_parser_common.h"

using namespace softwarecontainer;

class DeviceNodeTest : public GatewayParserCommon<std::string>
{
public:
    Device dev;
};

/*
 * Make sure configuring with just a device node name works
 */
TEST_F(DeviceNodeTest, TestConfigJustName) {
    const std::string config = "{ \"name\": \"/dev/random\" }";
    json_t *configJSON = convertToJSON(config);
    Device dev;

    ASSERT_TRUE(dev.parse(configJSON));
    ASSERT_FALSE(dev.getName().empty());
    ASSERT_EQ(dev.getMode(), -1);
}

/*
 * Test that a full proper config works.
 */
TEST_F(DeviceNodeTest, TestFullConfig) {
    const std::string config1 = "{\
                                    \"name\":  \"/dev/new_device\",\
                                    \"mode\":  644\
                                }";
    json_t *configJSON1 = convertToJSON(config1);
    const std::string config2 = "{\
                                    \"name\":  \"/dev/new_device\",\
                                    \"mode\":  764\
                                }";

    ASSERT_TRUE(dev.parse(configJSON1));

    json_t *configJSON2 = convertToJSON(config2);
    ASSERT_TRUE(dev.parse(configJSON2));
    ASSERT_FALSE(dev.getName().empty());
    ASSERT_EQ(dev.getMode(), 764);
}

/*
 * Test that setting mode fails if name is missing
 */
TEST_F(DeviceNodeTest, TestConfigNoName) {
    const std::string config = "{ \"mode\": 644 }";

    json_t *configJSON = convertToJSON(config);

    ASSERT_FALSE(dev.parse(configJSON));
    ASSERT_TRUE(dev.getName().empty());
}

/*
 * Test that setting mode of bad type fails
 */
TEST_F(DeviceNodeTest, TestConfigBadMode) {
    const std::string config = "{ \"name\": \"TEST_DEVICE\", \
                                  \"mode\": \"C\"\
                                }";
    json_t *configJSON = convertToJSON(config);

    ASSERT_FALSE(dev.parse(configJSON));
    ASSERT_EQ(dev.getMode(), -1);
}


class MockContainer : public ContainerAbstractInterface
{
public :
    MockContainer () {}
    const char *id() const {return "";}

    bool initialize() {return true;}
    bool create() {return true;}
    bool start(pid_t *) {return true;}
    bool stop() {return true;}

    bool shutdown() {return true;}
    bool shutdown(unsigned int) {return true;}

    bool suspend() {return true;}
    bool resume() {return true;}

    bool destroy() {return true;}
    bool destroy(unsigned int) {return true;}

    bool mountDevice(const std::string &) {return true;}
    bool createSymLink(const std::string &, const std::string &) {return true;}

    bool bindMountInContainer(const std::string &, const std::string &, bool ) {return true;}

    bool setEnvironmentVariable(const std::string &, const std::string &) {return true;}
    bool setCgroupItem(std::string, std::string) {return true;}

    bool execute(ExecFunction, pid_t *, const EnvironmentVariables &, int, int, int) {return false;}
    bool execute(const std::string &,
                 pid_t *,
                 const EnvironmentVariables &,
                 const std::string &,
                 int,
                 int,
                 int ) {return false;}
};


/*
 * Tests activate stage of the device
 */
TEST_F(DeviceNodeTest, Activation) {
    std::shared_ptr<ContainerAbstractInterface> container;
    container = std::shared_ptr<ContainerAbstractInterface> (new MockContainer());

    const std::string config = "{\"name\": \"/dev/input/mouse0\", \"mode\":  622}";
    json_t *configJSON = convertToJSON(config);

    ASSERT_TRUE(dev.parse(configJSON));
    ASSERT_EQ(dev.getMode(), 622);
    ASSERT_FALSE(dev.getIsconfigured());
    ASSERT_TRUE(dev.activate(container));
    ASSERT_TRUE(dev.getIsconfigured());
}



struct testSetup
{
    Device firstConfiguration;
    Device secondConfiguration;
    bool  shouldReconfigure;
};

class DynamicDeviceConfigurationTests : public  ::testing::TestWithParam<testSetup>
{
public:
    DeviceNodeLogic dnl;
    std::shared_ptr<ContainerAbstractInterface> container;

    void SetUp() override
    {
        container = std::shared_ptr<ContainerAbstractInterface> (new MockContainer());
    }
};

/*
 * This data is fed to the DynamicDeviceConfigurationTests tests
 *
 * The first set examines the case when new mode is configured for same device
 * The second set examines when a new device is added to configuration
 * The third set examines when exactly same device with same mode is added
 */
INSTANTIATE_TEST_CASE_P(DeviceModeConfigurations, DynamicDeviceConfigurationTests, ::testing::Values(
        testSetup{
            {"/dev/input/mouse0", 622},
            {"/dev/input/mouse0", 755},
            false
        },
        testSetup{
            {"/dev/tty0", 622},
            {"/dev/tty1", 652},
            true
        },
        testSetup{
            {"/dev/tty1", 622},
            {"/dev/tty1", 622},
            true
        }
));

TEST_P(DynamicDeviceConfigurationTests, DoubleActivation) {
    testSetup testparams = GetParam();
    //add first device to the device list of DeviceNodeLogic
    ASSERT_TRUE(dnl.updateDeviceList(testparams.firstConfiguration));

    //configure device
    auto devlist = dnl.getDevList();
    for (auto &dev : devlist) {
        dev->activate(container);
    }

    //add second device to the device list of DeviceNodeLogic
    ASSERT_TRUE(dnl.updateDeviceList(testparams.secondConfiguration));
    auto item = dnl.findDeviceByName(testparams.firstConfiguration.getName());
    ASSERT_NE(nullptr, item);
    ASSERT_EQ(testparams.shouldReconfigure, item->getIsconfigured());
}


struct testModes
{
    int itemMode;
    int newMode;
    int expectedMode;
};

class DeviceModeTests : public  ::testing::TestWithParam<testModes>
{
public:
    Device dev;
};

/*
 * This data is fed to the DeviceMode tests
 */
INSTANTIATE_TEST_CASE_P(DeviceModeParameters, DeviceModeTests, ::testing::Values(
        testModes{644, 777 ,777},
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
    auto testparams = GetParam();
    dev.setMode(testparams.itemMode);
    dev.calculateDeviceMode(testparams.newMode);
    ASSERT_EQ(testparams.expectedMode, dev.getMode());
}
