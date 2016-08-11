/*
 *   Copyright (C) 2016 Pelagicore AB
 *   All rights reserved.
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
        SoftwareContainerLibTest::SetUp();
    }

    const std::string NAME  = "Environment_variable_test_name";
    const std::string VALUE = "Environment_variable_test_value";
};


TEST_F(EnvironmentGatewayTest, TestActivateWithNoConf) {
    givenContainerIsSet(gw);
    ASSERT_FALSE(gw->activate());
}

TEST_F(EnvironmentGatewayTest, TestActivateWithEmptyConf) {
    givenContainerIsSet(gw);
    const std::string config = "[]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

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
