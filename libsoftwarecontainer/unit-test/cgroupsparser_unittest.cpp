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

#include "gateway/cgroups/cgroupsparser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class CGroupsParserTest : public ::testing::TestWithParam<std::string>
{
public:
    CGroupsParser parser;
    CGroupsParser::CGroupsPair result;
    json_error_t err;
    json_t *configJSON;

    void SetUp() override
    {
        std::string config = GetParam();
        configJSON = json_loads(config.c_str(), 0, &err);
        ASSERT_TRUE(configJSON != NULL);
    }

    void TearDown() override
    {
        if (configJSON) {
            free(configJSON);
        }
    }
};

/*
 * This is a parameterized test that makes sure that configuration fails when given bad
 * configurations.
 */
typedef CGroupsParserTest CGroupsNegativeTest;
TEST_P(CGroupsNegativeTest, FailsWhenConfigIsBad) {
    ASSERT_EQ(ReturnCode::FAILURE, parser.parseCGroupsGatewayConfiguration(configJSON, result));
    ASSERT_TRUE(result.first.empty());
    ASSERT_TRUE(result.second.empty());
}

/*
 * This is a parameterized test that makes sure that configuration succeeds when given
 * correct config data.
 */
typedef CGroupsParserTest CGroupsPositiveTest;
TEST_P(CGroupsPositiveTest, SuccessWhenConfigIsGood) {
    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseCGroupsGatewayConfiguration(configJSON, result));
    ASSERT_FALSE(result.first.empty());
    ASSERT_FALSE(result.second.empty());
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
       \"setting\": \"cpu.shares\"\
    }",

    // Value is wrong type
    "{\
       \"setting\": \"cpu.shares\",\
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
