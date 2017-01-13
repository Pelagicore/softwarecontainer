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

#include "softwarecontainer_test.h"
#include "gateway/devicenode/devicenodegateway.h"
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

    const std::string NEW_DEVICE = "/tmp/thenewfile";
    const std::string PRESENT_DEVICE = "/tmp/random";
};

/*
 * Make sure activation of the gateway works with a valid conf and a container
 */
TEST_F(DeviceNodeGatewayTest, TestActivateWithValidConf) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\":  \"" + NEW_DEVICE + "\",\
                                    \"major\": 4,\
                                    \"minor\": 32,\
                                    \"mode\":  644\
                                  },\
                                  {\
                                    \"name\": \"" + PRESENT_DEVICE + "\",\
                                    \"mode\":  \"654\"\
                                  },\
                                  {\
                                    \"name\": \"" + PRESENT_DEVICE + "\",\
                                    \"mode\":  \"645\"\
                                  }\
                                ]";

    ASSERT_TRUE(isSuccess(gw->setConfig(config)));
    ASSERT_TRUE(isSuccess(gw->activate()));
}

