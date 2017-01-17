/*
 * Copyright (C) 2017 Pelagicore AB
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gateway/network/networkgatewayfunctions.h"
#include "gateway/network/networkgateway.h"

using namespace softwarecontainer;

struct testParams
{
    uint32_t    netmask;
    std::string gatewayIP;
    int32_t     containerID;
    bool        expectSuccess;
    std::string expectedIP;
};

class NetworkGatewayFunctionsTests : public  ::testing::TestWithParam<testParams>
{
public:
    NetworkGatewayFunctions ngf;
    testParams testparams;

    void SetUp() override
    {
        testparams = GetParam();
    }
};

/*
 * This data is fed to the NetworkGatewayFunctions tests.
 *
 * To be able to understand these tests, it is important to be aware of following facts :
 *
 * IP address can not be same as gateway IP
 * IP address uniqueness is acquired by using (gatewayIP + containerID + 1).
 * The network is limited to generate IP address in a range of 2 ^ (32 - netmask)
 *
 * Thus there could be (2^netmask) - 1 available IP address.
 */
INSTANTIATE_TEST_CASE_P(NetworkFunctionsParameters, NetworkGatewayFunctionsTests, ::testing::Values(
        // no ip is available for 32 bit netmask
        testParams{32, "192.168.0.0", 0, false, ""},

        // for 31 bits mask there will be only one bit (32-31 =1), thus only use of 1 bit there can
        // be only one option to be generated
        testParams{31, "192.168.0.0", 0, true, "192.168.0.1"},
        testParams{31, "192.168.0.0", 1, false, ""},

        // for 30 bits mask there will be only 2 bits. Which means there can only 3 IP addresses to
        // be generated (192.168.0.0 is reserved for gateway ip)
        testParams{30, "192.168.0.0", 2, true, "192.168.0.3"},
        testParams{30, "192.168.0.0", 3, false, ""},
        testParams{30, "192.168.0.0", 4, false, ""},

        // for 29 bits mask there will be only 3 bits. Which means there can only 7 IP addresses to
        // be generated
        testParams{29, "192.168.0.0", 6, true, "192.168.0.7"},
        testParams{29, "192.168.0.0", 7, false, ""},
        testParams{29, "192.168.0.0", 8, false, ""},

        // for 28 bits mask there will be only 4 bits. Which means there can only 15 IP addresses to
        // be generated
        testParams{28, "192.168.0.0", 14, true, "192.168.0.15"},
        testParams{28, "192.168.0.0", 15, false, ""},
        testParams{28, "192.168.0.0", 16, false, ""},

        // for 27 bits mask there will be only 5 bits. Which means there can only 31 IP addresses to
        // be generated
        testParams{27, "192.168.0.0", 30, true, "192.168.0.31"},
        testParams{27, "192.168.0.0", 31, false, ""},
        testParams{27, "192.168.0.0", 32, false, ""},

        //For 16 bit bitmask (2^16)-1 different ip addres can be assigned due to containerID
        testParams{16, "192.168.0.0", 1024, true, "192.168.4.1"},
        testParams{16, "192.168.0.0", 65534, true, "192.168.255.255"},
        testParams{16, "192.168.0.0", 65535, false, ""},
        testParams{16, "192.168.0.0", 65536, false, ""},

        //For 24 bit bitmask (2^8)-1 different ip addres can be assigned due to containerID
        testParams{24, "192.168.0.0", 0, true, "192.168.0.1"},
        testParams{24, "192.168.0.0", 1, true, "192.168.0.2"},
        testParams{24, "192.168.0.0", 10, true, "192.168.0.11"},
        testParams{24, "192.168.0.0", 100, true, "192.168.0.101"},
        testParams{24, "192.168.0.0", 255, false, ""}
));

/*
 * Verify generateIP function works as ir should be
 */
TEST_P(NetworkGatewayFunctionsTests, IPAllocation) {
    if (!testparams.expectSuccess) {
        EXPECT_THROW(ngf.generateIP(testparams.netmask, testparams.gatewayIP, testparams.containerID),
                     IPAllocationError);
    } else {
        char convertionTmp[INET_ADDRSTRLEN];
        uint32_t internetAddress = 0;
        EXPECT_NO_THROW(internetAddress =
                        ngf.generateIP(testparams.netmask, testparams.gatewayIP, testparams.containerID));

        // convert ip to human readable form
        inet_ntop(AF_INET, &internetAddress, convertionTmp, INET_ADDRSTRLEN);
        ASSERT_EQ(convertionTmp, testparams.expectedIP);
    }
}
