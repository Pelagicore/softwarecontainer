/*
 *   Copyright (C) 2016 Pelagicore AB
 *   All rights reserved.
 */

#include <thread>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "generators.h"
#include "libpelagicontain.h"

#include "gateway/cgroupsgateway.h"

class CgroupsGatewayTest :
    public::testing::Test
{

public:
    CgroupsGatewayTest() { }

    void SetUp() override
    {
        ::testing::Test::SetUp();
        cgw = new CgroupsGateway();

        lib = std::unique_ptr<PelagicontainLib>(new PelagicontainLib());
        lib->setContainerIDPrefix("Test-");
        lib->setMainLoopContext(m_context);
        ASSERT_TRUE(isSuccess(lib->init()));
    }

    void TearDown() override
    {
        ::testing::Test::TearDown();
        lib->shutdown();
        lib.reset();
        delete cgw;
    }

    void givenContainerIsSet() {
        lib->getPelagicontain().addGateway(*cgw);
    }

    CgroupsGateway *cgw;
    ReturnCode container_create_result = ReturnCode::FAILURE;

    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    Glib::RefPtr<Glib::MainLoop> m_ml;
    std::unique_ptr<PelagicontainLib> lib;
};




TEST_F(CgroupsGatewayTest, TestActivateWithNoConf) {

    givenContainerIsSet();
    ASSERT_FALSE(cgw->activate());

}

TEST_F(CgroupsGatewayTest, TestActivateWithEmptyValidJSONConf) {
    givenContainerIsSet();
    const std::string config = "[]";

    ASSERT_TRUE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
}

TEST_F(CgroupsGatewayTest, TestActivateWithNoContainer) {
    const std::string config = "[\
                                  {\
                                    \"setting\": \"cpu.shares\",\
                                    \"value\": \"256\"\
                                  }\
                               ]";

    ASSERT_TRUE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
}

TEST_F(CgroupsGatewayTest, TestActivateWithValidConf) {
    givenContainerIsSet();
    const std::string config = "[\
                                  {\
                                    \"setting\": \"cpu.shares\",\
                                    \"value\": \"256\"\
                                  }\
                               ]";


    ASSERT_TRUE(cgw->setConfig(config));
    ASSERT_TRUE(cgw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithInvalidJSON) {
    givenContainerIsSet();

    std::string config = "hasdlaskndldn";
    ASSERT_FALSE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithNoneJSONObjects) {
    givenContainerIsSet();
    const std::string config = "[\
                 123,\
              ]";
    ASSERT_FALSE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
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
    ASSERT_FALSE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
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
    ASSERT_FALSE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithSettingMissing) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"setting\": [\"a\", \"b\"],\
                   \"value\": \"256\"\
                 }\
              ]";
    ASSERT_FALSE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithSettingNotString) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"setting\": [\"a\", \"b\"],\
                   \"value\": \"256\"\
                 }\
              ]";
    ASSERT_FALSE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithValueMissing) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"setting\": \"cpu.shares\",\
                 }\
              ]";
    ASSERT_FALSE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
}

TEST_F(CgroupsGatewayTest, TestSetConfigWithValueNotString) {
    givenContainerIsSet();
    const std::string config = "[\
                 {\
                   \"setting\": \"cpu.shares\",\
                   \"value\": [\"a\", \"b\"],\
                 }\
              ]";
    ASSERT_FALSE(cgw->setConfig(config));
    ASSERT_FALSE(cgw->activate());
}
