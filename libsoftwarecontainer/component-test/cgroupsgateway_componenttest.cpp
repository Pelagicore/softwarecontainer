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
#include "gateway/cgroups/cgroupsgateway.h"

#include "jansson.h"

class CgroupsGatewayTest : public SoftwareContainerGatewayTest
{

public:
    CgroupsGatewayTest() { }
    CgroupsGateway *gw;

    void SetUp() override
    {
        gw = new CgroupsGateway();
        SoftwareContainerTest::SetUp();
    }
};

/*
 * Test that activating the gateway, given a valid configuration, works.
 */
TEST_F(CgroupsGatewayTest, ActivateWithValidConf) {
    givenContainerIsSet(gw);
    const std::string config = "[{\
                                    \"setting\": \"memory.limit_in_bytes\",\
                                    \"value\": \"2000000\"\
                                }]";
    loadConfig(config);

    ASSERT_TRUE(isSuccess(gw->setConfig(jsonConfig)));
    ASSERT_TRUE(isSuccess(gw->activate()));
}
