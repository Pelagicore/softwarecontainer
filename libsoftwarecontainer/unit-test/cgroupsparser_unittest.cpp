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

#include "gateway/cgroups/cgroupsparser.h"
#include "gateway/cgroups/cgroupsgateway.h"
#include "gateway_parser_common.h"

using namespace softwarecontainer;

class CGroupsParserTest : public GatewayParserCommon<std::string>
{
public:
    CGroupsParser parser;

    std::string config;

    void SetUp() override
    {
        config = GetParam();
    }
};

/*
 * This is a parameterized test that makes sure that configuration fails when given bad
 * configurations.
 */
typedef CGroupsParserTest CGroupsNegativeTest;
TEST_P(CGroupsNegativeTest, FailsWhenConfigIsBad) {
    json_t *configJSON = convertToJSON(config);
    EXPECT_THROW(parser.parseCGroupsGatewayConfiguration(configJSON), CgroupsGatewayError);
    ASSERT_TRUE(parser.getSettings().empty());
}

/*
 * This is a parameterized test that makes sure that configuration succeeds when given
 * correct config data.
 */
typedef CGroupsParserTest CGroupsPositiveTest;
TEST_P(CGroupsPositiveTest, SuccessWhenConfigIsGood) {
    json_t *configJSON = convertToJSON(config);
    EXPECT_NO_THROW(parser.parseCGroupsGatewayConfiguration(configJSON));
    ASSERT_FALSE(parser.getSettings().empty());
}

/*
 * This data is fed to the NegativeTest
 */
INSTANTIATE_TEST_CASE_P(TestBadConfigs, CGroupsNegativeTest, ::testing::Values(
    // Missing setting
    "{ \"value\": \"256\" }",

    // Setting is wrong type
    "{\
        \"setting\": [\"a\", \"b\"],\
        \"value\": \"256\"\
    }",

    // No value
    "{\
       \"setting\": \"memory.limit_in_bytes\"\
    }",

    // Value is wrong type
    "{\
       \"setting\": \"memory.limit_in_bytes\",\
       \"value\": [\"a\", \"b\"]\
     }",

    // Value has a bad suffix
    "{\
       \"setting\": \"memory.limit_in_bytes\",\
       \"value\": \"15Q\"\
     }",

    // Value must be at least 2
    "{\
       \"setting\": \"cpu.shares\",\
       \"value\": \"1\"\
     }",

    // Too large number for cpu.shares, more than 64 bits
    "{\
       \"setting\": \"cpu.shares\",\
       \"value\": \"9223372036854775809\"\
     }",

    // Value must be at least 2
    "{\
       \"setting\": \"cpu.shares\",\
       \"value\": \"-1000\"\
     }",

    // Value must be an integer
    "{\
       \"setting\": \"cpu.shares\",\
       \"value\": \"abc\"\
     }",

    // Value is wrong type
    "{\
        \"setting\": \"net_cls.classid\",\
        \"value\": []\
    }",
    // Value is not proper hex value
    "{\
        \"setting\": \"net_cls.classid\",\
        \"value\": \"123\"\
    }",
    // Value is not proper hex value
    "{\
        \"setting\": \"net_cls.classid\",\
        \"value\": \"0x08PGAH\"\
    }",
    // Value is too short
    "{\
        \"setting\": \"net_cls.classid\",\
        \"value\": \"0xAAAA\"\
    }",
    // Value is too long
    "{\
        \"setting\": \"net_cls.classid\",\
        \"value\": \"0xAAAABBBBCCCC\"\
    }"
));

/*
 * This data is fed to the PositiveTest
 */
INSTANTIATE_TEST_CASE_P(TestGoodConfigs, CGroupsPositiveTest, ::testing::Values(
    "{\
        \"value\": \"test\",\
        \"setting\": \"test\"\
     }",

    // Just a normal value
    "{\
       \"setting\": \"memory.limit_in_bytes\",\
       \"value\": \"1500\"\
     }",

    // Value with proper suffix
    "{\
       \"setting\": \"memory.limit_in_bytes\",\
       \"value\": \"15k\"\
     }",

    // Value with proper suffix
    "{\
       \"setting\": \"memory.limit_in_bytes\",\
       \"value\": \"15m\"\
     }",

    // Value with proper suffix
    "{\
       \"setting\": \"memory.limit_in_bytes\",\
       \"value\": \"15g\"\
     }",

    // Proper value for cpu.shares
    "{\
       \"setting\": \"cpu.shares\",\
       \"value\": \"250\"\
     }",

    // Proper value for cpu.shares, on lower limit
    "{\
       \"setting\": \"cpu.shares\",\
       \"value\": \"2\"\
     }",

    // Very big number for cpu.shares
    "{\
       \"setting\": \"cpu.shares\",\
       \"value\": \"500000000\"\
     }",

    // Proper value for net_cls.classid
    "{\
        \"setting\": \"net_cls.classid\",\
        \"value\": \"0xAAAABBBB\"\
    }",

    // Proper value for net_cls.cla
    "{\
        \"setting\": \"net_cls.classid\",\
        \"value\": \"0xAAAAB\"\
    }"

));

struct testWhitelist
{
    std::string key;
    std::string firstConfig;
    std::string nextConfig;
    std::string expectedValue;
};

class CGroupsParserWhitelistTests : public GatewayParserCommon<testWhitelist>
{
public:
    CGroupsParser parser;
    testWhitelist testparams;

    void SetUp() override
    {
        testparams = GetParam();
    }
};

/*
 * This data is fed to the DeviceMode tests
 */
INSTANTIATE_TEST_CASE_P(CGroupsWhitelistParameters, CGroupsParserWhitelistTests, ::testing::Values(
        testWhitelist{
            "memory.limit_in_bytes",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"20\"}",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"10000\"}",
            "10000"
        },
        testWhitelist{
            "memory.limit_in_bytes",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"500\"}",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"10\"}",
            "500"
        },
        testWhitelist{
            "memory.limit_in_bytes",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"20K\"}",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"100K\"}",
            "102400"
        },
        testWhitelist{
            "memory.limit_in_bytes",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"100K\"}",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"1M\"}",
            "1048576"
        },

        testWhitelist{
            "memory.limit_in_bytes",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"1G\"}",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"10g\"}",
            "10737418240"
        },
        testWhitelist{
            "memory.limit_in_bytes",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"127m\"}",
            "{\"setting\": \"memory.limit_in_bytes\", \"value\": \"12g\"}",
            "12884901888"
        },
        testWhitelist{
            "memory.memsw.limit_in_bytes",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"20\"}",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"10000\"}",
            "10000"
        },
        testWhitelist{
            "memory.memsw.limit_in_bytes",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"500\"}",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"10\"}",
            "500"
        },
        testWhitelist{
            "memory.memsw.limit_in_bytes",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"20K\"}",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"100K\"}",
            "102400"
        },
        testWhitelist{
            "memory.memsw.limit_in_bytes",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"100K\"}",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"1M\"}",
            "1048576"
        },

        testWhitelist{
            "memory.memsw.limit_in_bytes",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"1G\"}",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"10g\"}",
            "10737418240"
        },
        testWhitelist{
            "memory.memsw.limit_in_bytes",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"127m\"}",
            "{\"setting\": \"memory.memsw.limit_in_bytes\", \"value\": \"12g\"}",
            "12884901888"
        },
        testWhitelist{
            "cpu.shares",
            "{\"setting\": \"cpu.shares\", \"value\": \"520\"}",
            "{\"setting\": \"cpu.shares\", \"value\": \"800\"}",
            "800"
        },
        testWhitelist{
            "cpu.shares",
            "{\"setting\": \"cpu.shares\", \"value\": \"3000\"}",
            "{\"setting\": \"cpu.shares\", \"value\": \"1500\"}",
            "3000"
        },
        testWhitelist{
            "unsupported.parameter",
            "{\"setting\": \"unsupported.parameter\", \"value\": \"500\"}",
            "{\"setting\": \"unsupported.parameter\", \"value\": \"10\"}",
            "10"
        },
        testWhitelist{
            "unsupported.parameter",
            "{\"setting\": \"unsupported.parameter\", \"value\": \"10\"}",
            "{\"setting\": \"unsupported.parameter\", \"value\": \"500\"}",
            "500"
        }
));

/*
 * Verify whitelisting with separate keys
 */
TEST_P(CGroupsParserWhitelistTests, WithRelevantKeys) {
    json_t *firstConfig = convertToJSON(testparams.firstConfig);
    EXPECT_NO_THROW(parser.parseCGroupsGatewayConfiguration(firstConfig));

    json_t *nextConfig = convertToJSON(testparams.nextConfig);
    EXPECT_NO_THROW(parser.parseCGroupsGatewayConfiguration(nextConfig));

    auto settings = parser.getSettings();
    ASSERT_EQ(settings[testparams.key], testparams.expectedValue);
}
