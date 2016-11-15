
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
#include "gateway/envgateway.h"
class EnvironmentGatewayTest : public SoftwareContainerGatewayTest
{

public:
    EnvironmentGateway *gw;

    void SetUp() override
    {
        gw = new EnvironmentGateway();
        SoftwareContainerTest::SetUp();
    }

    const std::string NAME  = "Environment_variable_test_name";
    const std::string VALUE = "Environment_variable_test_value";
};

TEST_F(EnvironmentGatewayTest, TestActivateWithEmptyJSonObjectConf) {
    givenContainerIsSet(gw);
    const std::string config = "[{}]";

    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(EnvironmentGatewayTest, TestActivateWithNoContainer) {
    const std::string config = "[\
                                  {\
                                    \"name\": \"" + NAME  + "\",\
                                    \"value\": \"" + VALUE + "\"\
                                  }\
                               ]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
    delete gw;
}

TEST_F(EnvironmentGatewayTest, TestActivateWithMinimumValidConf) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\": \"" + NAME  + "\",\
                                    \"value\": \"" + VALUE + "\"\
                                  }\
                               ]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());
}

TEST_F(EnvironmentGatewayTest, TestActivateWithAppend) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\": \"" + NAME  + "\",\
                                    \"value\": \"" + VALUE + "\",\
                                    \"append\": true\
                                  }\
                               ]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());
}

TEST_F(EnvironmentGatewayTest, TestActivateWithRepetedConfNoAppend) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                                  {\
                                    \"name\": \"" + NAME  + "\",\
                                    \"value\": \"" + VALUE + "\"\
                                  }\
                               ]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());
}
