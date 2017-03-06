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

#include "gateway/devicenode/devicenodeparser.h"
#include "gateway_parser_common.h"

using namespace softwarecontainer;

class DeviceNodeParserTest : public GatewayParserCommon<std::string>
{
public:
    DeviceNodeParser parser;
    DeviceNodeParser::Device result;
};

/*
 * Make sure configuring with just a device node name works
 */
TEST_F(DeviceNodeParserTest, TestConfigJustName) {
    const std::string config = "{ \"name\": \"/dev/random\" }";
    json_t *configJSON = convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_TRUE(parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_FALSE(dev.name.empty());
    ASSERT_EQ(dev.mode, -1);
}

/*
 * Test that a full proper config works.
 */
TEST_F(DeviceNodeParserTest, TestFullConfig) {
    const std::string config1 = "{\
                                    \"name\":  \"/dev/new_device\",\
                                    \"mode\":  644\
                                }";
    json_t *configJSON1 = convertToJSON(config1);
    DeviceNodeParser::Device dev;

    const std::string config2 = "{\
                                    \"name\":  \"/dev/new_device\",\
                                    \"mode\":  764\
                                }";

    ASSERT_TRUE(parser.parseDeviceNodeGatewayConfiguration(configJSON1, dev));

    json_t *configJSON2 = convertToJSON(config2);
    ASSERT_TRUE(parser.parseDeviceNodeGatewayConfiguration(configJSON2, dev));
    ASSERT_FALSE(dev.name.empty());
    ASSERT_NE(dev.mode, -1);
    ASSERT_EQ(dev.mode, 764);
}

/*
 * Test that setting mode fails if name is missing
 */
TEST_F(DeviceNodeParserTest, TestConfigNoName) {
    const std::string config = "{ \"mode\": 644 }";

    json_t *configJSON = convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_FALSE(parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_TRUE(dev.name.empty());
}

/*
 * Test that setting mode of bad type fails
 */
TEST_F(DeviceNodeParserTest, TestConfigBadMode) {
    const std::string config = "{ \"name\": \"TEST_DEVICE\", \
                                  \"mode\": \"C\"\
                                }";
    json_t *configJSON = convertToJSON(config);
    DeviceNodeParser::Device dev;

    ASSERT_FALSE(parser.parseDeviceNodeGatewayConfiguration(configJSON, dev));
    ASSERT_EQ(dev.mode, -1);
}
