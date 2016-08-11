/*
 *   Copyright (C) 2016 Pelagicore AB
 *   All rights reserved.
 */

#include "softwarecontainer_test.h"
#include "gateway/cgroupsgateway.h"

class CgroupsGatewayTest : public SoftwareContainerGatewayTest
{

public:
    CgroupsGatewayTest() { }
    CgroupsGateway *gw;

    void SetUp() override
    {
        gw = new CgroupsGateway();
        SoftwareContainerLibTest::SetUp();
    }
};

TEST_F(CgroupsGatewayTest, TestActivateWithNoConf) {

    givenContainerIsSet(gw);
    ASSERT_FALSE(gw->activate());

}

TEST_F(CgroupsGatewayTest, TestActivateWithEmptyValidJSONConf) {
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);

    std::string config = "hasdlaskndldn";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithNoneJSONObjects) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                 123,\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithPartiallyValidConfBefore) {
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);
    const std::string config = "[\
                 {\
                   \"value\": \"256\"\
                 }\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithSettingNotString) {
    givenContainerIsSet(gw);
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
    givenContainerIsSet(gw);
    const std::string config = "[\
                 {\
                   \"setting\": \"cpu.shares\",\
                 }\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithValueNotString) {
    givenContainerIsSet(gw);
    const std::string config = "[\
                 {\
                   \"setting\": \"cpu.shares\",\
                   \"value\": [\"a\", \"b\"],\
                 }\
              ]";
    ASSERT_FALSE(gw->setConfig(config));
    ASSERT_FALSE(gw->activate());
}
