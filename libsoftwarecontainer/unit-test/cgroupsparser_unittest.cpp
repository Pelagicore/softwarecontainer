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
    ASSERT_EQ(ReturnCode::FAILURE, parser.parseCGroupsGatewayConfiguration(configJSON));
    ASSERT_TRUE(parser.getSettings().empty());
}

/*
 * This is a parameterized test that makes sure that configuration succeeds when given
 * correct config data.
 */
typedef CGroupsParserTest CGroupsPositiveTest;
TEST_P(CGroupsPositiveTest, SuccessWhenConfigIsGood) {
    json_t *configJSON = convertToJSON(config);
    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseCGroupsGatewayConfiguration(configJSON));
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
     }"
));

/*
 * This data is fed to the PositiveTest
 */ 
INSTANTIATE_TEST_CASE_P(TestGoodConfigs, CGroupsPositiveTest, ::testing::Values(
    "{ \"value\": \"test\",\
        \"setting\": \"test\"\
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
 * Verify whitelisting with seperate keys
 */
TEST_P(CGroupsParserWhitelistTests, WithRelevantKeys) {
    json_t *firstConfig = convertToJSON(testparams.firstConfig);
    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseCGroupsGatewayConfiguration(firstConfig));

    json_t *nextConfig = convertToJSON(testparams.nextConfig);
    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseCGroupsGatewayConfiguration(nextConfig));

    auto settings = parser.getSettings();
    ASSERT_EQ(settings[testparams.key], testparams.expectedValue);
}
