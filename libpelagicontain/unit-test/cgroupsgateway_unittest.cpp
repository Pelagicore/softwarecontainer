/*
 *   Copyright (C) 2016 Pelagicore AB
 *   All rights reserved.
 */

#include "gateway_test.h"
#include "gateway/cgroupsgateway.h"

class CgroupsGatewayTest : public GatewayTest
{

public:
    CgroupsGatewayTest() { }

    void SetUp() override
    {
        gw = std::unique_ptr<Gateway>(new CgroupsGateway());
        GatewayTest::SetUp();
    }
};




TEST_F(CgroupsGatewayTest, TestActivateWithNoConf) {

    givenContainerIsSet();
    ASSERT_FALSE(gw->activate());

}

TEST_F(CgroupsGatewayTest, TestActivateWithEmptyValidJSONConf) {
    givenContainerIsSet();
    const std::string config = "[]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestActivateWithNoContainer) {
    const std::string config = "[\
                                  {\
                                    \"setting\": \"cpu.shares\",\
                                    \"value\": \"256\"\
                                  }\
                               ]";

    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestActivateWithValidConf) {
    givenContainerIsSet();
    const std::string config = "[\
                                  {\
                                    \"setting\": \"cpu.shares\",\
                                    \"value\": \"256\"\
                                  }\
                               ]";


    ASSERT_TRUE(gw->setConfig(config));
    ASSERT_TRUE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithInvalidJSON) {
    givenContainerIsSet();

    std::string config = "hasdlaskndldn";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithNoneJSONObjects) {
    givenContainerIsSet();
    const std::string config = "[\
                 123,\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithPartiallyValidConfBefore) {
    givenContainerIsSet();
    const std::string config = "[\
                 123,\
                 {\
                   \"setting\": \"cpu.shares\",\
                   \"value\": \"256\"\
                 }\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithPartiallyValidConfAfter) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"setting\": \"cpu.shares\",\
                   \"value\": \"256\"\
                 },\
                 123\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithSettingMissing) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"value\": \"256\"\
                 }\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithSettingNotString) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"setting\": [\"a\", \"b\"],\
                   \"value\": \"256\"\
                 }\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithValueMissing) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"setting\": \"cpu.shares\",\
                 }\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithValueNotString) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"setting\": \"cpu.shares\",\
                   \"value\": [\"a\", \"b\"],\
                 }\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}
