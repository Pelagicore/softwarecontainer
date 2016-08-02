/*
 *   Copyright (C) 2016 Pelagicore AB
 *   All rights reserved.
 */
#include "gateway_test.h"
#include "gateway/envgateway.h"
class EnvironmentGatewayTest : public GatewayTest
{

public:
    EnvironmentGatewayTest(): GatewayTest(){ }

    void SetUp() override
    {
        gw = std::unique_ptr<Gateway>(new EnvironmentGateway());
        GatewayTest::SetUp();
    }

    const std::string NAME  = "Environment_variable_test_name";
    const std::string VALUE = "Environment_variable_test_value";
};


TEST_F(EnvironmentGatewayTest, TestActivateWithNoConf) {
    givenContainerIsSet();
    ASSERT_FALSE(gw->activate());
}

TEST_F(EnvironmentGatewayTest, TestActivateWithEmptyConf) {
    givenContainerIsSet();
    const std::string config = "[]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(EnvironmentGatewayTest, TestActivateWithEmptyJSonObjectConf) {
    givenContainerIsSet();
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
}

TEST_F(EnvironmentGatewayTest, TestActivateWithMinimumValidConf) {
    givenContainerIsSet();
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
    givenContainerIsSet();
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
    givenContainerIsSet();
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
