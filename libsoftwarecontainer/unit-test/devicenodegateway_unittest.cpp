
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

#include "softwarecontainer_test.h"
#include "gateway/devicenodegateway.h"
#include <unistd.h>

class DeviceNodeGatewayTest : public SoftwareContainerGatewayTest
{
public:
    DeviceNodeGatewayTest() { }
    DeviceNodeGateway *gw;

    void SetUp() override
    {
        gw = new DeviceNodeGateway();
        SoftwareContainerTest::SetUp();
    }

    const std::string NEW_DEVICE = "/dev/new_device";
    const std::string PRESENT_DEVICE = "/dev/random";
};

TEST_F(DeviceNodeGatewayTest, TestActivateWithNoContainer) {
    const std::string config = "[\
                                  {\
                                    \"name\": \"" + PRESENT_DEVICE + "\"\
                                  }\
                                ]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(DeviceNodeGatewayTest, TestActivateWithValidConf) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\":  \"" + NEW_DEVICE + "\",\
                                    \"major\": \"1\",\
                                    \"minor\": \"0\",\
                                    \"mode\":  \"644\"\
                                  },\
                                  {\
                                    \"name\": \"" + PRESENT_DEVICE + "\"\
                                  },\
                                  {\
                                    \"name\": \"" + PRESENT_DEVICE + "\",\
                                    \"mode\":  \"644\"\
                                  }\
                                ]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());
}

TEST_F(DeviceNodeGatewayTest, TestConfigNoMajor) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\":  \"" + NEW_DEVICE + "\",\
                                    \"minor\": \"0\",\
                                    \"mode\":  \"644\"\
                                  }\
                                ]";

    ASSERT_FALSE(gw->setConfig(config));
}

TEST_F(DeviceNodeGatewayTest, TestConfigNoMinor) {

    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\":  \"" + NEW_DEVICE + "\",\
                                    \"major\": \"1\",\
                                    \"mode\":  \"644\"\
                                  }\
                                ]";

    ASSERT_FALSE(gw->setConfig(config));
}

TEST_F(DeviceNodeGatewayTest, TestConfigNoMode) {

    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\":  \"" + NEW_DEVICE + "\",\
                                    \"major\": \"1\",\
                                    \"minor\": \"0\"\
                                  }\
                                ]";

    ASSERT_FALSE(gw->setConfig(config));
}

TEST_F(DeviceNodeGatewayTest, TestConfigBadMajor) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\":  \"" + NEW_DEVICE + "\",\
                                    \"major\": \"A\",\
                                    \"minor\": \"0\",\
                                    \"mode\":  \"644\"\
                                  }\
                                ]";

    ASSERT_FALSE(gw->setConfig(config));
}

TEST_F(DeviceNodeGatewayTest, TestConfigBadMinor) {

    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\":  \"" + NEW_DEVICE + "\",\
                                    \"major\": \"1\",\
                                    \"minor\": \"A\",\
                                    \"mode\":  \"644\"\
                                  }\
                                ]";

    ASSERT_FALSE(gw->setConfig(config));
}

TEST_F(DeviceNodeGatewayTest, TestConfigBadMode) {

    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\":  \"" + NEW_DEVICE + "\",\
                                    \"major\": \"1\",\
                                    \"minor\": \"0\",\
                                    \"mode\":  \"A\"\
                                  }\
                                ]";

    ASSERT_FALSE(gw->setConfig(config));
}
