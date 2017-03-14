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

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "functionjob.h"
#include "softwarecontainer_test.h"
#include "gateway/devicenode/devicenodegateway.h"

class DeviceNodeGatewayTest : public SoftwareContainerGatewayTest
{
public:
    DeviceNodeGatewayTest() { }

    std::unique_ptr<DeviceNodeGateway> gw;

    void SetUp() override
    {
        SoftwareContainerGatewayTest::SetUp();
        gw = std::unique_ptr<DeviceNodeGateway>(new DeviceNodeGateway(m_container));
    }

    const std::string PRESENT_DEVICE = "/dev/random";
};

/*
 * Make sure activation of the gateway works with a valid conf and a container
 */
TEST_F(DeviceNodeGatewayTest, TestActivateWithValidConf) {
    const std::string config = "[\
                                  {\
                                    \"name\": \"" + PRESENT_DEVICE + "\",\
                                    \"mode\":  654\
                                  }\
                                ]";

    loadConfig(config);
    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_TRUE(gw->activate());
}

/*
 * Make sure we overwrite a device that already exists in the container with a more premissive mode
 */
TEST_F(DeviceNodeGatewayTest, TestOverwriteDeviceFails) {

    const std::string device = PRESENT_DEVICE;
    const std::string config = "[\
                                  {\
                                    \"name\": \"" + device + "\",\
                                    \"mode\":  644\
                                  },\
                                  {\
                                    \"name\": \"" + device + "\",\
                                    \"mode\":  622\
                                  },\
                                  {\
                                    \"name\": \"" + device + "\",\
                                    \"mode\":  777\
                                  }\
                                ]";

    loadConfig(config);
    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_TRUE(gw->activate());

    auto checkMode = FunctionJob(m_container, [device] () {
        struct stat st;
        const int readResult = stat(device.c_str(), &st);

        if (0 != readResult) {
            printf("Could not stat %s, %s\n", device.c_str(), strerror(errno));
            return readResult;
        }

        return st.st_mode & 777 ? 0 : 1;
    });

    checkMode.start();
    checkMode.wait();
    ASSERT_TRUE(checkMode.isSuccess());
}

struct testSetup
{
    const std::string firstConfiguration;
    const std::string secondConfiguration;
    const std::string deviceNameToExamine;
    bool  shouldReconfigure;
};

class DynamicConfigurationTests :
        public SoftwareContainerGatewayTest,
        public  ::testing::WithParamInterface<testSetup>
{
public:
    std::unique_ptr<DeviceNodeGateway> gw;

    void SetUp() override
    {
        SoftwareContainerGatewayTest::SetUp();
        gw = std::unique_ptr<DeviceNodeGateway>(new DeviceNodeGateway(m_container));
    }
};

/*
 * This data is fed to the DynamicConfiguration tests
 *
 * The first set examines the case when new mode is configured for same device
 * The second set examines when a new device is added to configuration
 * The third set examines when exactly same device with same mode is added
 */
INSTANTIATE_TEST_CASE_P(DeviceModeConfigurations, DynamicConfigurationTests, ::testing::Values(
        testSetup{
            "[{\"name\": \"/dev/tty0\", \"mode\":  622}]",
            "[{\"name\": \"/dev/tty0\", \"mode\":  755}]",
            "/dev/tty0",
            false
        },
        testSetup{
            "[{\"name\": \"/dev/tty0\", \"mode\":  622}]",
            "[{\"name\": \"/dev/tty1\", \"mode\":  652}]",
            "/dev/tty0",
            true
        },
        testSetup{
            "[{\"name\": \"/dev/tty1\", \"mode\":  622}]",
            "[{\"name\": \"/dev/tty1\", \"mode\":  622}]",
            "/dev/tty1",
            true
        }
));
/*
 * This tests stands for examining multiple activation feature of DeviceNodeGateway
 *
 */
TEST_P(DynamicConfigurationTests, DynamicDeviceNodeConfiguration) {

    testSetup testparams = GetParam();

    loadConfig(testparams.firstConfiguration);
    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_TRUE(gw->activate());

    loadConfig(testparams.secondConfiguration);
    ASSERT_TRUE(gw->setConfig(jsonConfig));
    ASSERT_EQ(testparams.shouldReconfigure, gw->isDeviceConfigured(testparams.deviceNameToExamine));
    ASSERT_TRUE(gw->activate());
}
