
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


#include "gateway/devicenode/devicenodeparser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class DeviceNodeParserTest : public ::testing::TestWithParam<std::string>
{
public:
    DeviceNodeParser parser;
    DeviceNodeParser::Device result;

    json_error_t err;
    json_t *configJSON;

    void SetUp() override
    {
    }

    void convertToJSON(const std::string config)
    {
        configJSON = json_loads(config.c_str(), 0, &err);
        ASSERT_TRUE(configJSON != NULL);
    }

    void TearDown()
    {
        if (configJSON) {
            json_decref(configJSON);
        }
    }

};

/*
 * Make sure configuring with just a device node name works
 */
TEST_F(DeviceNodeParserTest, TestConfigJustName) {
    const std::string config = "{ \"name\": \"/dev/random\" }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_FALSE(dev.name.empty());
    ASSERT_EQ(dev.major, -1);
    ASSERT_EQ(dev.minor, -1);
    ASSERT_EQ(dev.mode, -1);
}

/*
 * Test that a full proper config works.
 */
TEST_F(DeviceNodeParserTest, TestFullConfig) {
    const std::string config = "{\
                                    \"name\":  \"/dev/new_device\",\
                                    \"major\": 1,\
                                    \"minor\": 0,\
                                    \"mode\":  644\
                                }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    const std::string config2 = "{\
                                    \"name\":  \"/dev/new_device\",\
                                    \"major\": 1,\
                                    \"minor\": 0,\
                                    \"mode\":  764\
                                }";

    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    convertToJSON(config2);
    ASSERT_EQ(ReturnCode::SUCCESS, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_FALSE(dev.name.empty());
    ASSERT_NE(dev.major, -1);
    ASSERT_NE(dev.minor, -1);
    ASSERT_NE(dev.mode, -1);
    ASSERT_EQ(dev.mode, 764);
}

/*
 * Test that setting major/minor/mode fails if name is missing
 */
TEST_F(DeviceNodeParserTest, TestConfigNoName) {
    const std::string config = "{ \"major\": 2,\
                                  \"minor\": 0,\
                                  \"mode\": 644\
                                }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_TRUE(dev.name.empty());
}

/*
 * Test that setting minor/mode fails of major is missing
 */
TEST_F(DeviceNodeParserTest, TestConfigNoMajor) {
    const std::string config = "{ \"name\": \"TEST_DEVICE\", \
                                  \"minor\": 0,\
                                  \"mode\": 644\
                                }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_EQ(dev.major, -1);
}

/*
 * Test that setting major/mode fails if minor is missing
 */
TEST_F(DeviceNodeParserTest, TestConfigNoMinor) {
    const std::string config = "{ \"name\": \"TEST_DEVICE\", \
                                  \"major\": 0,\
                                  \"mode\": 644\
                                }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_EQ(dev.minor, -1);
}

/*
 * Test that setting major/minor fails if no mode is set.
 */
TEST_F(DeviceNodeParserTest, TestConfigNoMode) {
    const std::string config = "{ \"name\": \"TEST_DEVICE\", \
                                  \"major\": 0,\
                                  \"minor\": 6\
                                }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_EQ(dev.mode, -1);
}

/*
 * Test that setting major of bad type fails
 */
TEST_F(DeviceNodeParserTest, TestConfigBadMajor) {
    const std::string config = "{ \"name\": \"TEST_DEVICE\", \
                                  \"major\": \"A\",\
                                  \"minor\": 6,\
                                  \"mode\": 644\
                                }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_EQ(dev.major, -1);
}

/*
 * Test that setting minor of bad type fails
 */
TEST_F(DeviceNodeParserTest, TestConfigBadMainor) {
    const std::string config = "{ \"name\": \"TEST_DEVICE\", \
                                  \"major\": 2,\
                                  \"minor\": \"B\",\
                                  \"mode\": 644\
                                }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_EQ(dev.minor, -1);
}

/*
 * Test that setting mode of bad type fails
 */
TEST_F(DeviceNodeParserTest, TestConfigBadMode) {
    const std::string config = "{ \"name\": \"TEST_DEVICE\", \
                                  \"major\": 2,\
                                  \"minor\": 6,\
                                  \"mode\": \"C\"\
                                }";
    convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_EQ(ReturnCode::FAILURE, parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_EQ(dev.mode, -1);
}
