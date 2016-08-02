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

#include "gateway/envgateway.h"

class EnvironmentGatewayTest :
    public::testing::Test
{

public:
    EnvironmentGatewayTest() { }

    void SetUp() override
    {
        ::testing::Test::SetUp();
        gw = std::unique_ptr<EnvironmentGateway>(new EnvironmentGateway());

        workspace = std::unique_ptr<PelagicontainWorkspace>(new PelagicontainWorkspace());
        lib = std::unique_ptr<PelagicontainLib>(new PelagicontainLib(*workspace));
        lib->setContainerIDPrefix("Test-");
        lib->setMainLoopContext(m_context);
        ASSERT_TRUE(isSuccess(lib->init()));
    }

    void TearDown() override
    {
        ::testing::Test::TearDown();

        // Ensuring that the reset is done in correct order
        lib.reset();
        gw.reset();
        workspace.reset();
    }

    void givenContainerIsSet() {
        lib->getPelagicontain().addGateway(*gw);
    }

    Glib::RefPtr<Glib::MainContext> m_context = Glib::MainContext::get_default();
    std::unique_ptr<EnvironmentGateway> gw;
    std::unique_ptr<PelagicontainLib> lib;
    std::unique_ptr<PelagicontainWorkspace> workspace;

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
